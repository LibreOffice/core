using System.Buffers.Binary;
using XRay.Office.Text.Fonts;
using Shouldly;

namespace XRay.Office.Text.Tests;

/// <summary>
/// A fallback candidate that covers a character but cannot draw it is passed over.
/// </summary>
/// <remarks>
/// <para>
/// The floor under the fallback search. Coverage is a <c>cmap</c> question and drawing is not: a
/// face can map a character and keep its shape in a table this renderer does not read, and the
/// search would then end on a face that produces an empty run — the right family at the right
/// advance, drawing nothing, which is worse for a reader than a wrong glyph.
/// </para>
/// <para>
/// The faces here are made by stripping tables out of installed ones, because no installed face
/// has the defect: it is a floor rather than a fix, and the fix for the one colour face on this
/// machine is <see cref="ColourBitmaps"/>. Stripping is also the sharper instrument — the same
/// font with and without its outlines isolates the guard from every other difference between two
/// faces.
/// </para>
/// </remarks>
public class GlyphFallbackPaintabilityTests
{
    /// <summary>A face with no outlines and no strikes cannot paint, however well it covers.</summary>
    [Fact]
    public void AFaceStrippedOfItsOutlinesCannotPaint()
    {
        OpenTypeFace whole = Face(Path("LiberationSans-Regular.ttf"));
        OpenTypeFace stripped = Stripped(Path("LiberationSans-Regular.ttf"), "glyf", "loca");

        // The premise: stripping changed nothing about coverage, only about drawing.
        whole.Characters.Covers('A').ShouldBeTrue();
        stripped.Characters.Covers('A').ShouldBeTrue();

        GlyphPainting.CanPaintCharacter(whole, 'A').ShouldBeTrue();
        GlyphPainting.CanPaintCharacter(stripped, 'A').ShouldBeFalse();
    }

    /// <summary>
    /// The search moves on to the next candidate rather than answering with the one that cannot draw.
    /// </summary>
    /// <remarks>
    /// With no fontconfig configuration the search is LibreOffice's own fixed list, on which
    /// <c>dejavusans</c> comes before <c>liberationsans</c> — so a DejaVu Sans that cannot draw is
    /// offered first and passed over, and Liberation Sans is what comes back. Preferences are pinned
    /// to <see cref="FontconfigPreferences.None"/> so the order under test is the ported one and not
    /// this machine's.
    /// </remarks>
    [Fact]
    public void AnUnpaintableCandidateFallsThroughToTheNext()
    {
        string directory = Directory.CreateTempSubdirectory("xray-office-paint-").FullName;

        try
        {
            File.WriteAllBytes(
                System.IO.Path.Combine(directory, "DejaVuSans.ttf"),
                Strip(File.ReadAllBytes(Path("DejaVuSans.ttf")), "glyf", "loca"));
            File.Copy(Path("LiberationSans-Regular.ttf"),
                System.IO.Path.Combine(directory, "LiberationSans-Regular.ttf"));

            SystemFontResolver resolver = new(
                SystemFontIndex.Build([directory]), FontconfigPreferences.None);

            OpenTypeFace? found = resolver.FallbackFor('A');

            found.ShouldNotBeNull(
                "the crippled DejaVu Sans is first on LibreOffice's list and Liberation Sans is on it too");
            found!.FamilyName.ShouldBe("Liberation Sans");

            // The control on the premise: with its outlines left in, that same DejaVu Sans is what
            // the same search answers — so the assertion above is about the guard and not about the
            // order, which has not moved.
            string control = Directory.CreateTempSubdirectory("xray-office-paint-").FullName;

            try
            {
                File.Copy(Path("DejaVuSans.ttf"), System.IO.Path.Combine(control, "DejaVuSans.ttf"));
                File.Copy(Path("LiberationSans-Regular.ttf"),
                    System.IO.Path.Combine(control, "LiberationSans-Regular.ttf"));

                new SystemFontResolver(SystemFontIndex.Build([control]), FontconfigPreferences.None)
                    .FallbackFor('A')!.FamilyName.ShouldBe("DejaVu Sans");
            }
            finally
            {
                Directory.Delete(control, recursive: true);
            }
        }
        finally
        {
            Directory.Delete(directory, recursive: true);
        }
    }

    /// <summary>
    /// With nothing else installed the search answers nothing rather than an unpaintable face.
    /// </summary>
    /// <remarks>
    /// Nothing is the honest answer and it is also the useful one: <c>FontItemiser</c> reports an
    /// unresolved fallback and the primary face draws its own missing-glyph box, which is ink a
    /// reader can see and a diagnostic a caller can read. Answering with the face instead produced
    /// neither.
    /// </remarks>
    [Fact]
    public void NothingIsAnsweredWhenEveryCandidateIsUnpaintable()
    {
        string directory = Directory.CreateTempSubdirectory("xray-office-paint-").FullName;

        try
        {
            File.WriteAllBytes(
                System.IO.Path.Combine(directory, "DejaVuSans.ttf"),
                Strip(File.ReadAllBytes(Path("DejaVuSans.ttf")), "glyf", "loca"));

            SystemFontResolver resolver = new(
                SystemFontIndex.Build([directory]), FontconfigPreferences.None);

            // The control: the same index with the outlines left in does answer.
            SystemFontResolver whole = new(
                SystemFontIndex.Build([System.IO.Path.GetDirectoryName(Path("DejaVuSans.ttf"))!]),
                FontconfigPreferences.None);

            whole.FallbackFor('A').ShouldNotBeNull();
            resolver.FallbackFor('A').ShouldBeNull();
        }
        finally
        {
            Directory.Delete(directory, recursive: true);
        }
    }

    private static string Path(string fileName)
    {
        string[] directories =
        [
            "/usr/share/fonts/truetype/liberation",
            "/usr/share/fonts/truetype/liberation2",
            "/usr/share/fonts/truetype/dejavu",
            "/usr/share/fonts/TTF",
        ];

        string? found = directories
            .Select(directory => System.IO.Path.Combine(directory, fileName))
            .FirstOrDefault(File.Exists);

        Assert.SkipWhen(found is null, $"{fileName} is not installed; see check-env.sh");
        return found!;
    }

    private static OpenTypeFace Face(string path)
    {
        OpenTypeFace? face = OpenTypeFace.ReadFile(path);
        face.ShouldNotBeNull();
        return face!;
    }

    private static OpenTypeFace Stripped(string path, params string[] tables)
    {
        OpenTypeFace? face = OpenTypeFace.Read(Strip(File.ReadAllBytes(path), tables));
        face.ShouldNotBeNull();
        return face!;
    }

    /// <summary>
    /// The same font file with some tables removed from its directory.
    /// </summary>
    /// <remarks>
    /// The records are dropped and the table data left where it is, which is enough for any reader
    /// that goes through the directory — as every reader must, since the data has no other index.
    /// The checksums are left stale deliberately: nothing here verifies them, and rewriting them
    /// would be a second thing for this helper to get wrong.
    /// </remarks>
    private static byte[] Strip(byte[] font, params string[] tables)
    {
        int count = BinaryPrimitives.ReadUInt16BigEndian(font.AsSpan(4));
        List<byte[]> kept = [];

        for (int i = 0; i < count; i++)
        {
            byte[] record = font[(12 + (16 * i))..(12 + (16 * i) + 16)];
            string tag = System.Text.Encoding.ASCII.GetString(record, 0, 4);
            if (!tables.Contains(tag, StringComparer.Ordinal)) kept.Add(record);
        }

        byte[] stripped = (byte[])font.Clone();
        BinaryPrimitives.WriteUInt16BigEndian(stripped.AsSpan(4), (ushort)kept.Count);

        for (int i = 0; i < kept.Count; i++) kept[i].CopyTo(stripped, 12 + (16 * i));

        return stripped;
    }
}
