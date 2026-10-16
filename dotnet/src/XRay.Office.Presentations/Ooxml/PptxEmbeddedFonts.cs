using System.Xml.Linq;
using XRay.Office.Containers;
using XRay.Office.Core.Diagnostics;
using XRay.Office.Ooxml;
using XRay.Office.Text.Fonts;

namespace XRay.Office.Presentations.Ooxml;

/// <summary>
/// The faces a deck carries with it, from <c>p:embeddedFontLst</c>.
/// </summary>
/// <remarks>
/// <para>
/// A deck that embeds a face is precisely the deck where resolving by name gives the wrong
/// metrics: the author had the face installed and we do not, so every advance width, every line
/// break and every block height is measured against a stand-in. On
/// <c>Ramp Up Campaign - French.pptx</c> that stand-in was DejaVu Sans for Alegreya Sans, which
/// is wider — every block gained a line, five overprinted one another, and the last paragraph
/// was pushed off the slide edge. The gate saw only the 19 words that went with it, three steps
/// downstream of the cause.
/// </para>
/// <para>
/// <strong>Keyed on the <c>typeface</c> the entry declares, not on the family name inside the
/// face.</strong> The two disagree, and the run names the former: this deck embeds a face whose
/// own family is <c>Alegreya Sans</c> and declares it as <c>Alegreya Sans Regular Bold</c>, which
/// is what 54 runs ask for. LibreOffice arrives at the same answer from the other end — it
/// registers the face under its typographic family and then converts the legacy full name on the
/// run into that family (tdf#172647, <c>EmbeddedFontsManager::addEmbeddedFont</c>) — and the
/// observable result is the same face on the same runs, which is what the reference PDF shows:
/// <c>AlegreyaSans-Medium</c>, <c>AlegreyaSans-Bold</c> and <c>AlegreyaSans-ExtraBold</c>.
/// Matching on the declared name reproduces that without owning a legacy-name table.
/// </para>
/// <para>
/// Read lazily and decoded lazily, separately. A deck with an embedded font list pays for the
/// list when its first run asks for a face, and pays for a 260 KB part only if a run actually
/// names that entry — this deck embeds seven faces and draws with three.
/// </para>
/// </remarks>
internal sealed class PptxEmbeddedFonts
{
    /// <summary>A deck that embeds nothing, which is all but six of the slides track.</summary>
    public static PptxEmbeddedFonts None { get; } = new(new Dictionary<string, Entry>(StringComparer.OrdinalIgnoreCase));

    private readonly Dictionary<string, Entry> _byTypeface;

    private PptxEmbeddedFonts(Dictionary<string, Entry> byTypeface) => _byTypeface = byTypeface;

    /// <summary>Reads the deck's embedded font list, which is on the presentation part.</summary>
    public static PptxEmbeddedFonts Read(PptxFile file)
    {
        ArgumentNullException.ThrowIfNull(file);

        XElement? list = Ppt.Child(file.Presentation, "embeddedFontLst");
        if (list is null) return None;

        Dictionary<string, Entry> byTypeface = new(StringComparer.OrdinalIgnoreCase);

        foreach (XElement embedded in Ppt.Children(list, "embeddedFont"))
        {
            if (Ppt.Attribute(Ppt.Child(embedded, "font"), "typeface") is not { Length: > 0 } typeface)
                continue;

            List<Face> faces = [];
            Add(faces, embedded, "regular", weight: 400, italic: false);
            Add(faces, embedded, "bold", weight: 700, italic: false);
            Add(faces, embedded, "italic", weight: 400, italic: true);
            Add(faces, embedded, "boldItalic", weight: 700, italic: true);

            // A repeated typeface is malformed; the first wins, as everywhere else a deck repeats
            // a declaration it should have made once.
            if (faces.Count > 0) byTypeface.TryAdd(typeface, new Entry(file, faces));
        }

        return byTypeface.Count > 0 ? new PptxEmbeddedFonts(byTypeface) : None;

        static void Add(List<Face> faces, XElement embedded, string style, int weight, bool italic)
        {
            if (Ppt.RelationshipId(Ppt.Child(embedded, style)) is { Length: > 0 } id)
                faces.Add(new Face(id, weight, italic));
        }
    }

    /// <summary>
    /// The path of the embedded face a request should use, or null when the deck has none for it.
    /// </summary>
    /// <param name="typeface">The family the run names.</param>
    /// <param name="weight">The weight asked for, on the 1-1000 scale.</param>
    /// <param name="isItalic">Whether italic was asked for.</param>
    public string? FaceKeyFor(string? typeface, int weight, bool isItalic)
        => typeface is { Length: > 0 } named && _byTypeface.TryGetValue(named, out Entry? entry)
            ? entry.KeyFor(weight, isItalic)
            : null;

    /// <summary>One <c>p:embeddedFont</c>: a declared name and up to four relationship targets.</summary>
    private sealed class Entry(PptxFile file, List<Face> faces)
    {
        private readonly Dictionary<string, string?> _keys = new(StringComparer.Ordinal);

        public string? KeyFor(int weight, bool isItalic)
        {
            // The same scoring `SystemFontIndex.Best` uses over installed faces, so that choosing
            // among four embedded styles and choosing among four installed ones cannot disagree:
            // a wrong slant is worse than any weight gap, and among the rest the nearest weight
            // wins. An entry declaring only `p:regular` therefore answers it for a bold run, which
            // is what LibreOffice does too — it has the one face registered for that family and
            // emboldens it synthetically.
            Face? best = null;
            int bestScore = int.MaxValue;

            foreach (Face face in faces)
            {
                int score = (face.IsItalic == isItalic ? 0 : 10_000) + Math.Abs(face.Weight - weight);
                if (score >= bestScore) continue;

                bestScore = score;
                best = face;
            }

            return best is { } chosen ? Materialised(chosen.RelationshipId) : null;
        }

        /// <summary>
        /// Unwraps the part and writes the face out, once per relationship.
        /// </summary>
        /// <remarks>
        /// The null answer is cached alongside the successful one. A compressed container is the
        /// commonest reason for it — 18 of the slides track's 28 embedded parts are MicroType
        /// Express — and re-reading a quarter of a megabyte to reach the same conclusion on every
        /// run of every slide is the difference between a deck that renders and one that appears
        /// to hang.
        /// </remarks>
        private string? Materialised(string relationshipId)
        {
            if (_keys.TryGetValue(relationshipId, out string? cached)) return cached;

            string? key = null;

            if (file.Relationship(file.MainPartName, relationshipId) is { IsExternal: false } link
                && file.Package.GetPart(link.Target) is { } part)
            {
                key = KeyOf(part, link.Target);
            }

            _keys[relationshipId] = key;
            return key;
        }

        private string? KeyOf(IPackagePart part, string partName)
        {
            byte[] bytes;

            try
            {
                using Stream content = part.Open();
                using MemoryStream buffer = new();
                content.CopyTo(buffer);
                bytes = buffer.ToArray();
            }
            catch (Exception exception) when (exception is IOException or InvalidDataException)
            {
                Report(partName, "could not be read");
                return null;
            }

            if (EmbeddedOpenTypeFont.Read(bytes) is not { } embedded)
            {
                Report(partName, "is not an Embedded OpenType container");
                return null;
            }

            if (embedded.IsCompressed)
            {
                Report(partName, "is MicroType Express compressed, which this reader cannot decode");
                return null;
            }

            if (OpenTypeFace.Read(embedded.FontData.ToArray()) is null)
            {
                Report(partName, "does not hold a font this reader understands");
                return null;
            }

            return EmbeddedFontStore.Store(embedded.FontData.Span);
        }

        private void Report(string partName, string why)
            => file.Report(new Diagnostic(
                DiagnosticSeverity.Information, "PL2260",
                $"The embedded font part '{partName}' {why}, so the family it carries has been "
                + "resolved against the installed faces instead.",
                new DiagnosticLocation(partName)));
    }

    /// <summary>One style of one embedded family: which part holds it, and what it is.</summary>
    private readonly record struct Face(string RelationshipId, int Weight, bool IsItalic);
}
