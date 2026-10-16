using System.Buffers.Binary;
using System.Text;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Rendering.Pdf;
using XRay.Office.Text.Fonts;
using XRay.Office.Text.Shaping;
using Shouldly;

namespace XRay.Office.Rendering.Tests;

/// <summary>
/// Which tables an embedded font program keeps, and what happens to a face that carries an odd one.
/// </summary>
/// <remarks>
/// <para>
/// The subsetter used to name the tables it <em>dropped</em> — <c>GSUB</c>, <c>GPOS</c> and
/// <c>GDEF</c>, on the argument that shaping has already applied them. LibreOffice does the
/// opposite, and the difference is not only weight: it inverts the drop set and keeps fourteen
/// tags (<c>PhysicalFontFace::CreateFontSubset</c>,
/// <c>vcl/source/font/PhysicalFontFace.cxx</c>:546-562).
/// </para>
/// <para>
/// <strong>A zero-length <c>hdmx</c> makes <c>hb_subset_or_fail</c> fail outright</strong>, and a
/// failed subset is a face this writer names and does not embed — the PDF then draws the reader's
/// substitute for a family the document may have carried with it. Found on the four
/// <c>Font_Verdana_*.ttf</c> embedded in <c>Sean Monogue.odp</c>, whose <c>hdmx</c> and
/// <c>VDMX</c> are both present at length zero: subsetting returned null on all four, and
/// removing <c>hdmx</c> alone — not <c>VDMX</c>, not <c>LTSH</c> — made all four succeed.
/// </para>
/// <para>
/// The face is doctored here rather than shipped, because the point is a malformed table and a
/// corpus is for real documents. Everything else about it is a real installed face, so the
/// outlines the subsetter cuts down are real outlines.
/// </para>
/// </remarks>
public sealed class PdfSubsetTableTests
{
    private static readonly PdfRenderOptions Reproducible = new()
    {
        CreationDate = new DateTimeOffset(2024, 1, 1, 0, 0, 0, TimeSpan.Zero),
    };

    [Fact]
    public void AFaceCarryingAZeroLengthHdmxIsStillEmbedded()
    {
        if (!TestFace.IsAvailable) return;

        byte[] pdf = RenderWith(Doctored());

        PdfFile.Parse(pdf).FontPrograms().ShouldHaveSingleItem()
            .Length.ShouldBeGreaterThan(0,
                "a face whose subset fails is named and not embedded, and a reader then draws "
                + "its own substitute for a family the document carried");
    }

    [Fact]
    public void TheEmbeddedProgramHoldsOnlyTheTablesAPdfNeeds()
    {
        if (!TestFace.IsAvailable) return;

        byte[] program = PdfFile.Parse(RenderWith(Doctored())).FontPrograms().ShouldHaveSingleItem();
        HashSet<string> tags = Tags(program);

        // LibreOffice's own keep list, plus the identity `cmap` this writer substitutes for the
        // Unicode one hb-subset would have written: a PDF simple font addresses glyphs by
        // one-byte codes, not by code point.
        string[] admissible =
        [
            "head", "hhea", "hmtx", "loca", "maxp", "glyf", "CFF ",
            "post", "name", "OS/2", "cvt ", "fpgm", "prep", "CFF2", "cmap",
        ];

        tags.Except(admissible).ShouldBeEmpty();

        // And the malformed one is gone rather than carried through.
        tags.ShouldNotContain("hdmx");
    }

    // ------------------------------------------------------------------------------ fixtures

    /// <summary>An installed face with a zero-length <c>hdmx</c> spliced into its directory.</summary>
    private static string Doctored()
    {
        byte[] original = File.ReadAllBytes(TestFace.Reference.FaceKey);
        byte[] doctored = WithEmptyTable(original, "hdmx");

        string path = Path.Combine(
            Path.GetTempPath(),
            "xray-office-zero-hdmx-" + Environment.ProcessId + ".ttf");

        File.WriteAllBytes(path, doctored);
        return path;
    }

    /// <summary>
    /// Rebuilds an sfnt's table directory with one extra entry of length zero.
    /// </summary>
    /// <remarks>
    /// The table data is untouched and simply moves down by sixteen bytes; every offset is
    /// rewritten to match. The new record points at the end of the file with a length of zero,
    /// which is exactly the shape the Verdana faces carry and is legal enough that every reader
    /// tried loads the font.
    /// </remarks>
    private static byte[] WithEmptyTable(byte[] font, string tag)
    {
        int count = BinaryPrimitives.ReadUInt16BigEndian(font.AsSpan(4));
        List<(uint Tag, uint Checksum, int Offset, int Length)> tables = [];

        for (int i = 0; i < count; i++)
        {
            int record = 12 + (i * 16);
            tables.Add((
                BinaryPrimitives.ReadUInt32BigEndian(font.AsSpan(record)),
                BinaryPrimitives.ReadUInt32BigEndian(font.AsSpan(record + 4)),
                (int)BinaryPrimitives.ReadUInt32BigEndian(font.AsSpan(record + 8)),
                (int)BinaryPrimitives.ReadUInt32BigEndian(font.AsSpan(record + 12))));
        }

        uint added = BinaryPrimitives.ReadUInt32BigEndian(Encoding.ASCII.GetBytes(tag));
        tables.RemoveAll(entry => entry.Tag == added);
        tables.Add((added, 0, 0, 0));
        tables.Sort((a, b) => a.Tag.CompareTo(b.Tag));

        int directory = 12 + (tables.Count * 16);
        MemoryStream output = new();
        output.Write(font, 0, 4);
        Write16(output, (ushort)tables.Count);
        Write16(output, 0);
        Write16(output, 0);
        Write16(output, 0);

        List<byte[]> bodies = [];
        int cursor = directory;

        foreach ((uint entryTag, uint checksum, int offset, int length) in tables)
        {
            byte[] body = length == 0 ? [] : font.AsSpan(offset, length).ToArray();
            bodies.Add(body);

            Write32(output, entryTag);
            Write32(output, checksum);
            Write32(output, (uint)(length == 0 ? cursor : cursor));
            Write32(output, (uint)length);

            cursor += (body.Length + 3) & ~3;
        }

        foreach (byte[] body in bodies)
        {
            output.Write(body, 0, body.Length);
            for (int pad = (4 - (body.Length % 4)) % 4; pad > 0; pad--) output.WriteByte(0);
        }

        return output.ToArray();

        static void Write16(Stream stream, ushort value)
        {
            Span<byte> buffer = stackalloc byte[2];
            BinaryPrimitives.WriteUInt16BigEndian(buffer, value);
            stream.Write(buffer);
        }

        static void Write32(Stream stream, uint value)
        {
            Span<byte> buffer = stackalloc byte[4];
            BinaryPrimitives.WriteUInt32BigEndian(buffer, value);
            stream.Write(buffer);
        }
    }

    /// <summary>The four-character tags of an sfnt's tables.</summary>
    private static HashSet<string> Tags(byte[] font)
    {
        HashSet<string> tags = new(StringComparer.Ordinal);
        int count = BinaryPrimitives.ReadUInt16BigEndian(font.AsSpan(4));

        for (int i = 0; i < count; i++)
        {
            tags.Add(Encoding.ASCII.GetString(font, 12 + (i * 16), 4));
        }

        return tags;
    }

    /// <summary>A one-page PDF drawing one word in the face at a given path.</summary>
    private static byte[] RenderWith(string faceKey)
    {
        OpenTypeFace face = OpenTypeFace.ReadFile(faceKey).ShouldNotBeNull();

        FontReference reference = new()
        {
            FamilyName = face.FamilyName ?? "Doctored",
            RequestedFamily = face.FamilyName ?? "Doctored",
            Weight = 400,
            FaceKey = faceKey,
        };

        GlyphRun run = Shaped(face, reference, "Hamburgefonstiv");

        using MemoryStream output = new();
        new PdfRenderer(Reproducible).Render(
            new DrawnPages(new DrawnPage(
                DrawnPage.A4,
                sink => sink.DrawGlyphRun(run, Paint.Solid(Colour.Black)))),
            output);

        return output.ToArray();
    }

    private static GlyphRun Shaped(OpenTypeFace face, FontReference reference, string text)
    {
        Length size = Length.FromPoints(24);
        ShapedText shaped = TextShaper.Default.Shape(face, text);

        List<PositionedGlyph> glyphs = [];
        List<int> clusters = [];
        Length pen = Length.Zero;

        foreach (ShapedGlyph glyph in shaped.Glyphs)
        {
            Length advance = shaped.Scale(glyph.Advance, size);
            glyphs.Add(new PositionedGlyph(glyph.GlyphId, new DocPoint(pen, Length.Zero), advance));
            clusters.Add(glyph.Cluster);
            pen += advance;
        }

        return new GlyphRun
        {
            Font = reference,
            FontSize = size,
            Origin = new DocPoint(Length.FromPoints(72), Length.FromPoints(144)),
            Glyphs = glyphs,
            Text = text,
            ClusterMap = clusters,
        };
    }
}
