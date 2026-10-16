using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.Text.Fonts;
using Shouldly;

namespace XRay.Office.Text.Tests;

/// <summary>
/// Tests the <c>glyf</c> outline reader that WordArt draws through.
/// </summary>
/// <remarks>
/// <para>
/// An outline cannot be checked against a number the font states: <c>glyf</c> carries a bounding box
/// per glyph, and that is exactly what these assert against — the reader's own points must fill the
/// box the font declares for the same glyph, to within the rounding of a scale into EMUs. That is a
/// real check rather than a tautology, because the box is stored ahead of the points and a reader
/// that mis-decodes the delta coding produces points that do not fill it.
/// </para>
/// <para>
/// Liberation Sans because it is the face Arial resolves to, and Arial is what the WordArt catalogue
/// asks for; a machine without it skips.
/// </para>
/// </remarks>
public class GlyphOutlineTests
{
    private static OpenTypeFace Require()
    {
        string[] candidates =
        [
            "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
            "/usr/share/fonts/truetype/liberation2/LiberationSans-Regular.ttf",
        ];

        string? path = Array.Find(candidates, File.Exists);
        Assert.SkipWhen(path is null, "Liberation Sans is not installed; see check-env.sh");

        OpenTypeFace? face = OpenTypeFace.ReadFile(path!);
        face.ShouldNotBeNull();
        return face!;
    }

    [Fact]
    public void ATrueTypeFaceCanBeOutlined() => GlyphOutlines.CanOutline(Require()).ShouldBeTrue();

    /// <summary>A letter's outline fills the box its own <c>glyf</c> record declares.</summary>
    [Theory]
    [InlineData('O')]
    [InlineData('W')]
    [InlineData('A')]
    [InlineData('a')]
    public void ALetterFillsItsDeclaredBox(char letter)
    {
        OpenTypeFace face = Require();
        ushort glyph = face.Characters.GlyphFor(letter);
        glyph.ShouldNotBe((ushort)0);

        Length em = Length.FromPoints(100);
        GraphicsPath? path = GlyphOutlines.Of(face, glyph, em);
        path.ShouldNotBeNull();
        path!.Commands.Count.ShouldBeGreaterThan(4);

        (double left, double top, double right, double bottom) = Bounds(path);

        // The declared box, mapped the way the reader maps a point: scaled by em/upem and with y
        // flipped, so the font's yMax becomes the smallest y.
        (int xMin, int yMin, int xMax, int yMax) = DeclaredBox(face, glyph);
        double scale = em.Emu / (double)face.UnitsPerEm;

        left.ShouldBe(xMin * scale, 2000);
        right.ShouldBe(xMax * scale, 2000);
        top.ShouldBe(-yMax * scale, 2000);
        bottom.ShouldBe(-yMin * scale, 2000);
    }

    /// <summary>A blank glyph draws nothing, and that is not the same as being unreadable.</summary>
    [Fact]
    public void ASpaceOutlinesToAnEmptyPath()
    {
        OpenTypeFace face = Require();
        GraphicsPath? path = GlyphOutlines.Of(face, face.Characters.GlyphFor(' '), Length.FromPoints(100));

        path.ShouldNotBeNull();
        path!.Commands.ShouldBeEmpty();
    }

    /// <summary>An accented letter is a composite, and its parts are placed rather than dropped.</summary>
    [Fact]
    public void AnAccentedLetterCarriesBothItsParts()
    {
        OpenTypeFace face = Require();
        ushort plain = face.Characters.GlyphFor('e');
        ushort accented = face.Characters.GlyphFor('é');
        accented.ShouldNotBe((ushort)0);

        Length em = Length.FromPoints(100);
        GraphicsPath bare = GlyphOutlines.Of(face, plain, em)!;
        GraphicsPath composite = GlyphOutlines.Of(face, accented, em)!;

        // The accent adds contours above the letter, so the composite reaches higher and holds more.
        composite.Commands.Count.ShouldBeGreaterThan(bare.Commands.Count);
        Bounds(composite).Top.ShouldBeLessThan(Bounds(bare).Top);
    }

    private static (double Left, double Top, double Right, double Bottom) Bounds(GraphicsPath path)
    {
        double left = double.MaxValue;
        double top = double.MaxValue;
        double right = double.MinValue;
        double bottom = double.MinValue;

        foreach (PathCommand command in path.Commands)
        {
            if (command.Verb == PathVerb.Close) continue;

            Include(command.Point.X.Emu, command.Point.Y.Emu);
            if (command.Verb != PathVerb.CubicTo) continue;

            Include(command.Control1.X.Emu, command.Control1.Y.Emu);
            Include(command.Control2.X.Emu, command.Control2.Y.Emu);
        }

        return (left, top, right, bottom);

        void Include(double x, double y)
        {
            left = Math.Min(left, x);
            right = Math.Max(right, x);
            top = Math.Min(top, y);
            bottom = Math.Max(bottom, y);
        }
    }

    /// <summary>The bounding box the font states for a glyph, in design units.</summary>
    private static (int XMin, int YMin, int XMax, int YMax) DeclaredBox(OpenTypeFace face, ushort glyph)
    {
        ReadOnlySpan<byte> loca = face.File.Table("loca");
        ReadOnlySpan<byte> glyf = face.File.Table("glyf");
        bool isLong = face.Head.IndexToLocFormat != 0;
        int entry = isLong ? 4 : 2;
        int start = glyph * entry;

        long offset = isLong
            ? System.Buffers.Binary.BinaryPrimitives.ReadUInt32BigEndian(loca[start..])
            : System.Buffers.Binary.BinaryPrimitives.ReadUInt16BigEndian(loca[start..]) * 2L;

        ReadOnlySpan<byte> record = glyf[(int)offset..];
        return (
            System.Buffers.Binary.BinaryPrimitives.ReadInt16BigEndian(record[2..]),
            System.Buffers.Binary.BinaryPrimitives.ReadInt16BigEndian(record[4..]),
            System.Buffers.Binary.BinaryPrimitives.ReadInt16BigEndian(record[6..]),
            System.Buffers.Binary.BinaryPrimitives.ReadInt16BigEndian(record[8..]));
    }
}
