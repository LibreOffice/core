using System.Text;
using Paperless.Core.Graphics;

namespace Paperless.WordProcessing.Rtf;

/// <content>
/// <c>{\pict}</c>: the one place RTF stops being text.
/// </content>
/// <remarks>
/// <para>
/// A picture's bytes arrive in one of two forms and both are read here. The default is hexadecimal —
/// two ASCII characters per byte, wrapped across lines however the producer felt like — and the
/// alternative is <c>\binN</c>, whose <c>N</c> bytes follow the delimiter raw. The tokeniser handles the
/// second (see <see cref="RtfTokenKind.Binary"/>, where the consequence of not doing so is spelled out);
/// this half handles the first, and assembles either into the picture the frame draws.
/// </para>
/// <para>
/// The size comes from <c>\picwgoal</c>/<c>\pichgoal</c> when the file states them and from
/// <c>\picw</c>/<c>\pich</c> when it does not, then scaled by <c>\picscalex</c>/<c>\picscaley</c>. That
/// is <c>RTFDocumentImpl::resolvePict</c> exactly
/// (<c>sw/source/writerfilter/rtftok/rtfdocumentimpl.cxx:1173</c>):
/// <c>nXExt = (nGoalWidth ? nGoalWidth : nWidth)</c> and then the percentage. The order matters — a file
/// that states a goal <em>and</em> a scale means the scale to apply to the goal, and a reader that
/// preferred the raw pixel count would draw a picture at its own resolution.
/// </para>
/// </remarks>
public sealed partial class RtfDocumentReader
{
    /// <summary>The scale a picture that states none is drawn at, as a percentage.</summary>
    private const int UnscaledPicture = 100;

    /// <summary>The picture being read, or null when no <c>{\pict}</c> group is open.</summary>
    private PictureBuilder? _picture;

    /// <summary>Opens a picture, which stays open until its group closes.</summary>
    /// <remarks>
    /// The offset is taken now, while the anchoring paragraph is still open and before anything else has
    /// been appended to it — which makes it the position of the character the picture sits in front of,
    /// and therefore where the picture hangs on the line. RTF appends nothing to the text for a picture,
    /// unlike ODF and OOXML, whose readers put an anchor character there; the offset is the whole of the
    /// record either way.
    /// </remarks>
    private void BeginPicture()
        => _picture = new PictureBuilder { Depth = _groupDepth, Offset = CurrentFlow.LayoutLength };

    /// <summary>Handles a control word inside a <c>{\pict}</c>, or reports that it was not one.</summary>
    /// <remarks>
    /// The blip words are recorded but not believed: what the picture <em>is</em> comes from its bytes,
    /// as it does in all four formats. The word is kept only so that a picture in a format this library
    /// does not draw can be declined by the name the document used for it, which is the difference
    /// between a useful diagnostic and a shrug.
    /// </remarks>
    private bool HandlePictureWord(RtfToken token)
    {
        if (_picture is not { } picture) return false;

        switch (token.Name)
        {
            case "picw": picture.Width = token.Parameter ?? 0; return true;
            case "pich": picture.Height = token.Parameter ?? 0; return true;
            case "picwgoal": picture.GoalWidth = token.Parameter ?? 0; return true;
            case "pichgoal": picture.GoalHeight = token.Parameter ?? 0; return true;
            case "picscalex": picture.ScaleX = token.Parameter ?? UnscaledPicture; return true;
            case "picscaley": picture.ScaleY = token.Parameter ?? UnscaledPicture; return true;

            case "pngblip" or "jpegblip" or "emfblip" or "macpict" or "pmmetafile"
                or "wmetafile" or "dibitmap" or "wbitmap":
                picture.Kind = token.Name;
                return true;

            default:
                return false;
        }
    }

    /// <summary>Collects a picture's hexadecimal payload.</summary>
    /// <remarks>
    /// Character by character with everything that is not a hex digit ignored, because a producer wraps
    /// the payload at whatever column it likes and RTF's own escapes can appear in the middle of it. An
    /// odd number of digits at the end is dropped: half a byte is not a byte.
    /// </remarks>
    private void AppendPictureHex(string text)
    {
        if (_picture is not { } picture) return;

        foreach (char character in text)
        {
            int digit = character switch
            {
                >= '0' and <= '9' => character - '0',
                >= 'a' and <= 'f' => character - 'a' + 10,
                >= 'A' and <= 'F' => character - 'A' + 10,
                _ => -1,
            };

            if (digit < 0) continue;
            picture.AppendNibble(digit);
        }
    }

    /// <summary>Collects the raw payload of a <c>\binN</c>.</summary>
    private void AppendPictureBinary(ReadOnlySpan<byte> bytes) => _picture?.Append(bytes);

    /// <summary>
    /// Finishes the open picture and hands it to the paragraph it is anchored in.
    /// </summary>
    /// <remarks>
    /// As an as-character frame, which is what an RTF picture outside a <c>{\shp}</c> is: it sits in the
    /// run of text where it was written and takes room on that line. A picture with no bytes, or with
    /// bytes in a format this library cannot draw, still produces the frame — the room it reserves is
    /// what keeps the text around it where the document put it, and the diagnostic says why the
    /// rectangle is empty.
    /// </remarks>
    private void EndPicture()
    {
        if (_picture is not { } picture) return;
        _picture = null;

        (int width, int height) = picture.Twips;
        if (width <= 0 || height <= 0) return;

        FramePicture image = EmbeddedPicture.Read(
            picture.Bytes, picture.Kind is null ? null : "\\" + picture.Kind, "\\pict", _diagnostics);

        // A picture inside a shape's `pib` or `fillBlip` property belongs to that shape and is not a
        // second picture in the text. `fillBlip` only where `pib` said nothing: a picture frame states
        // both and they are the same bytes, but a shape merely *filled* with a picture states only the
        // second, and taking whichever arrived last would let the fill overwrite the frame.
        if (_shape is { } shape && _shapeProperty is "pib" or "fillBlip")
        {
            if (_shapeProperty == "pib" || shape.Picture.IsEmpty) shape.Picture = image;
            return;
        }

        CurrentFlow.PendingFrames.Add(new RtfLayoutFrame(
            picture.Offset, 0, 0, width, height, Wrap: 5, WrapSide: 0,
            HorizontalOrigin: null, VerticalOrigin: null, Blocks: [])
        {
            IsInline = true,
            Picture = image,
        });
    }

    /// <summary>
    /// A picture while its group is open: the numbers RTF states loosely, and the bytes.
    /// </summary>
    /// <remarks>
    /// Mutable and short-lived for the same reason <c>ShapeBuilder</c> is: RTF states a picture as a
    /// stream of control words with no structure between them, so there is nothing to construct from
    /// until the closing brace.
    /// </remarks>
    private sealed class PictureBuilder
    {
        /// <summary>
        /// How large a picture's payload may be before the rest is dropped.
        /// </summary>
        /// <remarks>
        /// A guard on untrusted input rather than a real limit — sixty-four megabytes is far past any
        /// picture a document sensibly carries, and a file claiming more is claiming it in a stream
        /// where the length was never declared.
        /// </remarks>
        private const int MaxBytes = 64 * 1024 * 1024;

        private readonly List<byte> _bytes = [];
        private int _pending = -1;

        public int Depth { get; init; }

        public int Offset { get; init; }

        /// <summary><c>\picw</c>: the picture's own width, in the units its format counts in.</summary>
        public int Width { get; set; }

        /// <summary><c>\pich</c>: its own height.</summary>
        public int Height { get; set; }

        /// <summary><c>\picwgoal</c>: the width it is to be drawn at, in twips.</summary>
        public int GoalWidth { get; set; }

        /// <summary><c>\pichgoal</c>: the height it is to be drawn at, in twips.</summary>
        public int GoalHeight { get; set; }

        /// <summary><c>\picscalex</c>, as a percentage of whichever width applies.</summary>
        public int ScaleX { get; set; } = UnscaledPicture;

        /// <summary><c>\picscaley</c>.</summary>
        public int ScaleY { get; set; } = UnscaledPicture;

        /// <summary>Which blip control word the document used, for a diagnostic.</summary>
        public string? Kind { get; set; }

        /// <summary>The payload assembled so far.</summary>
        public ReadOnlyMemory<byte> Bytes => _bytes.ToArray();

        /// <summary>
        /// The rectangle the picture is drawn in, in twips.
        /// </summary>
        /// <remarks>
        /// <c>resolvePict</c>'s rule: the goal wins where it is stated, and the scale applies to
        /// whichever won. <c>\picw</c> is left as the file wrote it — pixels for a bitmap, metafile
        /// units otherwise — which is what LibreOffice does too, and it is only ever reached by a file
        /// that states no goal at all.
        /// </remarks>
        public (int Width, int Height) Twips
            => (Scaled(GoalWidth != 0 ? GoalWidth : Width, ScaleX),
                Scaled(GoalHeight != 0 ? GoalHeight : Height, ScaleY));

        /// <summary>Adds one hexadecimal digit, emitting a byte on every second one.</summary>
        public void AppendNibble(int digit)
        {
            if (_pending < 0)
            {
                _pending = digit;
                return;
            }

            if (_bytes.Count < MaxBytes) _bytes.Add((byte)((_pending << 4) | digit));
            _pending = -1;
        }

        /// <summary>Adds raw bytes from a <c>\binN</c>.</summary>
        public void Append(ReadOnlySpan<byte> bytes)
        {
            int room = MaxBytes - _bytes.Count;
            if (room <= 0) return;

            _bytes.AddRange(bytes.Length <= room ? bytes : bytes[..room]);
        }

        private static int Scaled(int value, int percent)
            => percent == UnscaledPicture ? value : (int)((long)value * percent / 100);
    }
}
