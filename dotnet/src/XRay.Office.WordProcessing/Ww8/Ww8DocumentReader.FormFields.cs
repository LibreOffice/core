using System.Buffers.Binary;

namespace XRay.Office.WordProcessing.Ww8;

/// <content>
/// Legacy form fields. Only one of them draws: the <c>FORMCHECKBOX</c> square.
/// </content>
/// <remarks>
/// <para>
/// Word writes the control as <c>U+0013 " FORMCHECKBOX " U+0001 U+0015</c> — a field with no
/// separator and no cached result, whose <c>U+0001</c> placeholder therefore sits inside the
/// <em>instruction</em>. A walk that drops everything between a field's start and its separator drops
/// the box with it, which is what left every square off <c>1528364855.doc</c> (37 fields) and
/// <c>f111.doc</c> (58). Neither document has a single one with a <c>U+0014</c> in it, and the boxes
/// that <em>do</em> come out of <c>1528364855.doc</c> today are literal <c>U+25A1</c> characters in
/// the text rather than fields at all — which is why the same page shows some and not others.
/// </para>
/// <para>
/// The square's size is the line's text height and not anything the field states, exactly as it is on
/// the DOCX side: <c>SwFieldFormCheckboxPortion::Format</c> (<c>sw/source/core/text/portxt.cxx</c>:1492)
/// sets width and height to <c>rInf.GetTextHeight()</c>. So the <c>hps</c> in the <c>FFData</c> is read
/// by nothing here, the same way <c>w:checkBox/w:size</c> is inert — see
/// <c>DocxLayoutSource.CheckBoxFrame</c>, whose measurement settled it.
/// </para>
/// </remarks>
public sealed partial class Ww8DocumentReader
{
    /// <summary>
    /// How much of the <c>Data</c> stream a picture header takes before the <c>FFData</c> begins.
    /// </summary>
    /// <remarks>
    /// A <c>WW8_PIC</c>, which <c>ImportFormulaControl</c> reads whole before handing the stream to
    /// <c>WW8FormulaControl::FormulaRead</c> (<c>ww8par3.cxx</c>:2088-2096). Fixed rather than taken
    /// from the header's own <c>cbHeader</c>, because that is what LibreOffice does.
    /// </remarks>
    private const int PictureHeaderLength = 68;

    /// <summary>The <c>iRes</c> that means "no result is stored, show the default".</summary>
    /// <remarks><c>if (iRes != 25) mnChecked = iRes;</c>, <c>ww8par3.cxx</c>:2181.</remarks>
    private const int UnstatedResult = 25;

    /// <summary>Whether the checkbox whose placeholder sits at a character position is ticked.</summary>
    /// <remarks>
    /// The <c>FFData</c> is in the <c>Data</c> stream at the placeholder's own
    /// <c>sprmCPicLocation</c>, behind a picture header. Everything unreadable answers "not ticked":
    /// an empty square is what the box looks like when the file says nothing, and the alternative —
    /// declining to draw it — is the defect this exists to fix.
    /// </remarks>
    private bool IsCheckedBox(int position)
    {
        if (PictureLocation(position) is not { } offset) return false;
        if (offset < 0 || offset > _pictures.Length - PictureHeaderLength - 10) return false;

        ReadOnlySpan<byte> picture = _pictures.AsSpan(offset);
        if (BinaryPrimitives.ReadInt32LittleEndian(picture) <= MinimumPictureLength) return false;

        ReadOnlySpan<byte> data = picture[PictureHeaderLength..];

        // "An unsigned integer that MUST be 0xFFFFFFFF" — the FFData's own header.
        if (BinaryPrimitives.ReadUInt32LittleEndian(data) != 0xFFFFFFFF) return false;

        int result = (data[4] & 0x7C) >> 2;
        if (result != UnstatedResult) return result != 0;

        // The default sits after the two flag bytes, the two counts and the control's name.
        int at = 10;
        if (!SkipString(data, ref at)) return false;

        return at + 2 <= data.Length && BinaryPrimitives.ReadUInt16LittleEndian(data[at..]) != 0;
    }

    /// <summary>Steps over one <c>Xstz</c>: a count, that many characters, and a terminator.</summary>
    private static bool SkipString(ReadOnlySpan<byte> data, ref int at)
    {
        if (at < 0 || at + 2 > data.Length) return false;

        int characters = BinaryPrimitives.ReadUInt16LittleEndian(data[at..]);
        at += 2 + ((characters + 1) * 2);

        return at >= 0 && at <= data.Length;
    }

    /// <summary>A legacy form checkbox, and where in the paragraph's text its square sits.</summary>
    /// <param name="Offset">The anchor character it stands behind.</param>
    /// <param name="IsChecked">Whether the box is ticked, from the <c>FFData</c>'s <c>iRes</c>.</param>
    public readonly record struct Ww8LayoutCheckBox(int Offset, bool IsChecked);
}
