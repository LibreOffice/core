using System.Xml.Linq;
using XRay.Office.Core.Diagnostics;
using XRay.Office.OpenDocument;
using XRay.Office.OpenDocument.Styles;
using XRay.Office.Text.Fonts;

namespace XRay.Office.Presentations.OpenDocument;

/// <summary>
/// The faces an ODF document carries with it, from <c>svg:font-face-uri</c>.
/// </summary>
/// <remarks>
/// <para>
/// The ODF counterpart of <see cref="Ooxml.PptxEmbeddedFonts"/>, and it exists for the same
/// reason: a document that embeds a face is precisely the document where resolving by name gives
/// the wrong metrics, because the author had the face installed and we do not. Neither
/// <c>Verdana</c> nor <c>Alegreya Sans</c> exists on this machine — <c>fc-list</c> finds neither
/// and <c>fc-match</c> answers <c>DejaVuSans.ttf</c> for both — yet 26.2.4.2 embeds them in its
/// own PDF of <c>Sean Monogue.odp</c> and <c>Ramp Up Campaign - French.odp</c>, because it reads
/// them out of the document. <strong>This is the opposite of a font confound</strong>: the face
/// is inside the file, so both renderers have equal access to it and every divergence is ours.
/// </para>
/// <para>
/// <strong>The declaration is an <c>office:font-face-decls</c> entry with children, and only the
/// attributes were being read.</strong> A <c>style:font-face</c> that carries a face has one
/// <c>svg:font-face-src</c> holding one <c>svg:font-face-uri</c> per style, each naming a package
/// part through <c>xlink:href</c> — or, in flat XML, carrying the bytes as an
/// <c>office:binary-data</c> child. The importer's seat is
/// <c>XMLFontStyleContextFontFaceUri::endFastElement</c>
/// (<c>xmloff/source/style/XMLFontStylesContext.cxx</c>:250-312), which decides the container
/// from <c>svg:font-face-format</c> and hands the stream to
/// <c>SvXMLImport::addEmbeddedFont</c>.
/// </para>
/// <para>
/// <strong>The style is read out of the face and not off the declaration.</strong> LibreOffice's
/// export writes <c>loext:font-style</c> and <c>loext:font-weight</c> on every
/// <c>svg:font-face-uri</c>, and the importer reads neither — that <c>SetAttribute</c> handles
/// <c>xlink:href</c> and nothing else — so which of a family's four files is bold is decided by
/// the file's own <c>OS/2</c>. Reading the attributes instead would agree on every document
/// LibreOffice wrote and disagree with the reference on any that mislabels one.
/// </para>
/// <para>
/// <strong>Keyed on the family the declaration states, and on the face's own typographic family
/// when they differ.</strong> <c>EmbeddedFontsManager::addEmbeddedFont</c> registers the face
/// under <c>font.getTypographicFamilyName()</c> — name 16 falling back to name 1
/// (<c>vcl/source/font/TrueTypeFont.cxx</c>:140-145) — and only falls back to the document's own
/// name when the face carries none (<c>vcl/source/gdi/embeddedfontsmanager.cxx</c>:355-362,
/// tdf#172647). A run, meanwhile, names <c>svg:font-family</c>. Both names are indexed here
/// because the two coincide in every file LibreOffice writes and the second costs nothing once
/// the face is decoded.
/// </para>
/// <para>
/// Read lazily and decoded lazily, separately, exactly as the deck reader is: a document with no
/// embedded declaration pays a dictionary miss, and one that embeds nine faces and draws with
/// four decodes four.
/// </para>
/// </remarks>
internal sealed class OdfEmbeddedFonts
{
    /// <summary>A document that embeds nothing, which is 296 of the 302 converted <c>.odp</c>.</summary>
    public static OdfEmbeddedFonts None { get; } = new(null, []);

    private readonly OdfFile? _file;
    private readonly Dictionary<string, Family> _byFamily;

    private OdfEmbeddedFonts(OdfFile? file, Dictionary<string, Family> byFamily)
    {
        _file = file;
        _byFamily = byFamily;
    }

    /// <summary>Reads the document's embedded font declarations.</summary>
    public static OdfEmbeddedFonts Read(OdfFile file)
    {
        ArgumentNullException.ThrowIfNull(file);

        Dictionary<string, Family> byFamily = new(StringComparer.OrdinalIgnoreCase);

        foreach (OdfFontFace declaration in file.Styles.FontFaces.Values)
        {
            List<Source> sources = SourcesOf(declaration);
            if (sources.Count == 0) continue;

            // The family the declaration states, normalised the way a run's own lookup
            // normalises it: `svg:font-family` is a CSS list and may be quoted, and
            // `OdfTextBody.Family` hands the resolver its first entry unquoted. Keying on
            // anything else means the key and the request are different strings.
            string family = FamilyIn(declaration.FontFamily) ?? declaration.Name;
            if (family.Length == 0) continue;

            if (!byFamily.TryGetValue(family, out Family? entry))
            {
                byFamily[family] = entry = new Family(file);
            }

            entry.Add(sources);
        }

        return byFamily.Count > 0 ? new OdfEmbeddedFonts(file, byFamily) : None;
    }

    /// <summary>
    /// The path of the embedded face a request should use, or null when the document has none.
    /// </summary>
    /// <param name="family">The family the run names.</param>
    /// <param name="weight">The weight asked for, on the 1-1000 scale.</param>
    /// <param name="isItalic">Whether italic was asked for.</param>
    public string? FaceKeyFor(string? family, int weight, bool isItalic)
    {
        if (_file is null || family is not { Length: > 0 } named) return null;
        if (!_byFamily.TryGetValue(named, out Family? entry)) return null;

        // Decoding a family also learns what the faces call themselves, so a request naming the
        // typographic form of a family the document declared under a legacy name finds it on the
        // next lookup rather than never.
        string? key = entry.KeyFor(weight, isItalic);

        foreach (string alias in entry.SelfDeclaredNames)
        {
            if (!_byFamily.ContainsKey(alias)) _byFamily[alias] = entry;
        }

        return key;
    }

    /// <summary>
    /// The <c>svg:font-face-uri</c> children of one declaration, in the order they are written.
    /// </summary>
    private static List<Source> SourcesOf(OdfFontFace declaration)
    {
        List<Source> sources = [];

        foreach (XElement src in declaration.Element.Elements(
                     XName.Get("font-face-src", OdfNamespaces.SvgCompatible)))
        {
            foreach (XElement uri in src.Elements(
                         XName.Get("font-face-uri", OdfNamespaces.SvgCompatible)))
            {
                string? href = uri.Attribute(XName.Get("href", OdfNamespaces.XLink))?.Value;
                XElement? inline = uri.Element(XName.Get("binary-data", OdfNamespaces.Office));

                if (string.IsNullOrWhiteSpace(href) && inline is null) continue;

                string? format = uri
                    .Element(XName.Get("font-face-format", OdfNamespaces.SvgCompatible))
                    ?.Attribute(XName.Get("string", OdfNamespaces.SvgCompatible))?.Value;

                sources.Add(new Source(href, inline, format));
            }
        }

        return sources;
    }

    /// <summary>The first family of a CSS-style family list, unquoted, or null.</summary>
    private static string? FamilyIn(string? list)
    {
        if (string.IsNullOrWhiteSpace(list)) return null;

        int comma = list.IndexOf(',', StringComparison.Ordinal);
        string first = (comma >= 0 ? list[..comma] : list).Trim().Trim('\'', '"').Trim();

        return first.Length == 0 ? null : first;
    }

    /// <summary>One embedded family: the declarations that name it, decoded on first use.</summary>
    private sealed class Family(OdfFile file)
    {
        private readonly List<Source> _sources = [];
        private readonly List<Face> _faces = [];
        private readonly List<string> _names = [];
        private bool _decoded;

        /// <summary>What the decoded faces call themselves, once anything has asked.</summary>
        public IReadOnlyList<string> SelfDeclaredNames => _names;

        public void Add(IEnumerable<Source> sources)
        {
            foreach (Source source in sources)
            {
                // Two declarations routinely name one file -- `Sean Monogue.odp` declares both
                // `Verdana` and `Verdana1` over the same four parts -- and decoding it twice
                // would put the same face in the scoring twice.
                if (source.Href is { Length: > 0 } href
                    && _sources.Any(s => string.Equals(s.Href, href, StringComparison.Ordinal)))
                {
                    continue;
                }

                _sources.Add(source);
            }
        }

        public string? KeyFor(int weight, bool isItalic)
        {
            Decode();

            // The same scoring `PptxEmbeddedFonts` and `SystemFontIndex.Best` use, so that
            // choosing among embedded styles and choosing among installed ones cannot disagree:
            // a wrong slant costs more than any weight gap, and among the rest the nearest
            // weight wins.
            Face? best = null;
            int bestScore = int.MaxValue;

            foreach (Face face in _faces)
            {
                int score = (face.IsItalic == isItalic ? 0 : 10_000) + Math.Abs(face.Weight - weight);
                if (score >= bestScore) continue;

                bestScore = score;
                best = face;
            }

            return best?.Key;
        }

        private void Decode()
        {
            if (_decoded) return;
            _decoded = true;

            foreach (Source source in _sources)
            {
                if (Bytes(source) is not { Length: > 0 } raw) continue;

                // `svg:font-face-format`'s three CSS values, and LibreOffice's reading of them:
                // absent, `opentype` and `truetype` are a bare sfnt, `embedded-opentype` is an
                // EOT container, and anything else warns and is assumed to be a bare sfnt
                // (`XMLFontStylesContext.cxx`:241-273).
                bool eot = string.Equals(
                    source.Format, "embedded-opentype", StringComparison.OrdinalIgnoreCase);

                byte[]? sfnt = eot ? Unwrapped(raw, source) : raw;
                if (sfnt is null) continue;

                if (OpenTypeFace.Read(sfnt) is not { } face)
                {
                    Report(source, "does not hold a font this reader understands");
                    continue;
                }

                if (EmbeddedFontStore.Store(sfnt) is not { Length: > 0 } key)
                {
                    Report(source, "could not be written where it can be opened by path");
                    continue;
                }

                _faces.Add(new Face(key, face.Weight, face.IsItalic));

                if (face.FamilyName is { Length: > 0 } self && !_names.Contains(self))
                {
                    _names.Add(self);
                }
            }
        }

        /// <summary>The container's bytes, from the package part or from the inline base64.</summary>
        private byte[]? Bytes(Source source)
        {
            if (source.Inline is { } inline)
            {
                try
                {
                    return Convert.FromBase64String(inline.Value);
                }
                catch (FormatException)
                {
                    Report(source, "carries base64 that cannot be decoded");
                    return null;
                }
            }

            // A reference outside the package is not fetched, here as everywhere else: extraction
            // must make no network request, and doing so on untrusted input is an SSRF.
            // LibreOffice declines the same case -- "External URL for font file not handled."
            if (source.Href is not { Length: > 0 } href
                || href.Contains("://", StringComparison.Ordinal))
            {
                return null;
            }

            string part = href.StartsWith("./", StringComparison.Ordinal) ? href[2..] : href;

            try
            {
                using Stream? content = file.OpenPart(part);
                if (content is null) return null;

                using MemoryStream buffer = new();
                content.CopyTo(buffer);
                return buffer.ToArray();
            }
            catch (Exception exception) when (exception is IOException or InvalidDataException)
            {
                Report(source, "could not be read");
                return null;
            }
        }

        private byte[]? Unwrapped(byte[] raw, Source source)
        {
            if (EmbeddedOpenTypeFont.Read(raw) is not { } embedded)
            {
                Report(source, "is not an Embedded OpenType container");
                return null;
            }

            if (embedded.IsCompressed)
            {
                Report(source, "is MicroType Express compressed, which this reader cannot decode");
                return null;
            }

            return embedded.FontData.ToArray();
        }

        /// <summary>
        /// Records a face that could not be used, rather than failing the document.
        /// </summary>
        /// <remarks>
        /// A face that cannot be materialised costs the document a substitution, which is the
        /// ordinary outcome for a family nothing has — and not the document itself. That is the
        /// leniency rule applied to the one step here that touches a filesystem.
        /// </remarks>
        private void Report(Source source, string why)
            => file.Report(new Diagnostic(
                DiagnosticSeverity.Information, "PL2261",
                $"The embedded font '{source.Href ?? "(inline)"}' {why}, so the family it carries "
                + "has been resolved against the installed faces instead.",
                new DiagnosticLocation(source.Href ?? "content.xml")));
    }

    /// <summary>One <c>svg:font-face-uri</c>: where the bytes are and what wraps them.</summary>
    private readonly record struct Source(string? Href, XElement? Inline, string? Format);

    /// <summary>One decoded face: where it was written, and what it turned out to be.</summary>
    private readonly record struct Face(string Key, int Weight, bool IsItalic);
}
