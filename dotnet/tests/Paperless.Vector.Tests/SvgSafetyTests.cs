using System.Diagnostics;
using System.Globalization;
using System.IO.Compression;
using System.Text;
using Paperless.Core.Geometry;
using Paperless.Vector.Svg;
using Shouldly;

namespace Paperless.Vector.Tests;

/// <summary>
/// What an embedded vector image is not allowed to do.
/// </summary>
/// <remarks>
/// <para>
/// These are the tests that would have caught a live vulnerability, so they assert rather
/// than trust a default. With the SVG library's stock settings an
/// <c>&lt;image href="file:///etc/passwd"&gt;</c> caused that file's bytes to be read and
/// handed to the asset loader — measured, on this machine, against version 5.1.1. A library
/// upgrade could reintroduce that, and only a test will say so.
/// </para>
/// <para>
/// The work bounds are here for the same reason: a 1 057-byte <c>&lt;use&gt;</c> bomb hung
/// the library for over two minutes before the guard existed, and nothing about that file
/// looks unusual.
/// </para>
/// </remarks>
public sealed class SvgSafetyTests
{
    private static readonly SvgImageDecoder Decoder = new();

    [Fact]
    public void ALocalFileReferenceIsNotFetched()
    {
        // A real, readable file rather than a fabricated path: a reference that could not
        // have resolved proves nothing, because it would fail whether or not the fetch was
        // attempted. The bytes are a valid PNG, so a loader that did read them would
        // succeed — and the test would see the image it must not see.
        string leaked = Path.Combine(Path.GetTempPath(), $"paperless-svg-leak-{Guid.NewGuid():N}.png");
        File.WriteAllBytes(leaked, Convert.FromBase64String(Png));

        try
        {
            VectorImage image = Decode($"""
                <svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" width="200" height="100">
                  <image x="0" y="0" width="100" height="100" xlink:href="file://{leaked}"/>
                </svg>
                """);

            Recorder sink = new();
            image.Draw(sink, new DocRect(DocPoint.Origin, image.IntrinsicSize));

            sink.Images.ShouldBeEmpty();
        }
        finally
        {
            File.Delete(leaked);
        }
    }

    [Fact]
    public void AnHttpReferenceIsNotFetched()
    {
        VectorImage image = Decode("""
            <svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" width="200" height="100">
              <image x="0" y="0" width="100" height="100" xlink:href="https://example.invalid/pixel.png"/>
            </svg>
            """);

        Recorder sink = new();
        image.Draw(sink, new DocRect(DocPoint.Origin, image.IntrinsicSize));

        sink.Images.ShouldBeEmpty();
    }

    [Fact]
    public void AnExternalXmlEntityIsNotResolved()
    {
        // The classic XXE: a DOCTYPE declaring an entity that reads a file, referenced from
        // text that would then be drawn — and, in a rendered document, read.
        string secret = Path.Combine(Path.GetTempPath(), $"paperless-svg-xxe-{Guid.NewGuid():N}.txt");
        File.WriteAllText(secret, "TOPSECRET");

        try
        {
            VectorImage image = Decode($"""
                <?xml version="1.0"?>
                <!DOCTYPE svg [<!ENTITY leak SYSTEM "file://{secret}">]>
                <svg xmlns="http://www.w3.org/2000/svg" width="200" height="100">
                  <text x="10" y="20" font-size="12">&leak;</text>
                </svg>
                """);

            Recorder sink = new();
            image.Draw(sink, new DocRect(DocPoint.Origin, image.IntrinsicSize));

            foreach ((string text, _, _, _, _) in sink.Runs) text.ShouldNotContain("TOPSECRET");
        }
        finally
        {
            File.Delete(secret);
        }
    }

    [Fact]
    public void ScriptingIsInert()
    {
        // A static export shows the initial state, and nothing in it runs. The rectangles
        // still draw, which is what distinguishes "scripting ignored" from "document rejected".
        VectorImage image = Decode("""
            <svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" width="200" height="100">
              <script>document.title = 'x';</script>
              <rect x="0" y="0" width="10" height="10" fill="#ff0000" onclick="alert(1)"/>
              <a xlink:href="javascript:alert(2)"><rect x="20" y="0" width="10" height="10" fill="#0000ff"/></a>
            </svg>
            """);

        Recorder sink = new();
        image.Draw(sink, new DocRect(DocPoint.Origin, image.IntrinsicSize));

        sink.Fills.Count.ShouldBe(2);
    }

    [Fact]
    public void AUseBombIsRefusedInsteadOfExpanded()
    {
        // Thirteen levels of three references each: 3^13 ≈ 1.6 million nodes from 1 057 bytes.
        // Before the guard existed this had not finished after 120 seconds.
        string bomb = UseBomb(levels: 13, fanOut: 3);
        bomb.Length.ShouldBeLessThan(4096);

        Stopwatch clock = Stopwatch.StartNew();
        VectorImage image = Decode(bomb);
        clock.Stop();

        image.IsEmpty.ShouldBeTrue();
        image.Diagnostics.ShouldContain(diagnostic => diagnostic.Code == "PL6001");
        clock.Elapsed.ShouldBeLessThan(TimeSpan.FromSeconds(10));
    }

    [Fact]
    public void AModestUseTreeStillDraws()
    {
        // The guard must not be so eager that ordinary clip art stops working: a few hundred
        // instantiated nodes is normal for an icon set.
        VectorImage image = Decode(UseBomb(levels: 4, fanOut: 3));

        image.IsEmpty.ShouldBeFalse();
        image.Diagnostics.ShouldNotContain(diagnostic => diagnostic.Code == "PL6001");
    }

    [Fact]
    public void ACyclicUseIsRefused()
    {
        VectorImage image = Decode("""
            <svg xmlns="http://www.w3.org/2000/svg" width="100" height="100">
              <g id="a"><use href="#b"/></g>
              <g id="b"><use href="#a"/></g>
            </svg>
            """);

        image.IsEmpty.ShouldBeTrue();
        image.Diagnostics.ShouldContain(diagnostic => diagnostic.Code == "PL6001");
    }

    [Fact]
    public void ADocumentPastTheByteCapIsNotParsedAtAll()
    {
        VectorLimits tiny = VectorLimits.Default with { MaxBytes = 64 };

        VectorImage image = Decoder.Decode(
            Encoding.UTF8.GetBytes("""<svg xmlns="http://www.w3.org/2000/svg" width="100" height="100"><rect width="10" height="10"/></svg>"""),
            tiny);

        image.IsEmpty.ShouldBeTrue();
        image.Diagnostics.ShouldContain(diagnostic => diagnostic.Code == "PL6002");
    }

    [Fact]
    public void TheCommandCapTruncatesRatherThanHangs()
    {
        VectorLimits tiny = VectorLimits.Default with { MaxCommands = 4 };

        StringBuilder svg = new("""<svg xmlns="http://www.w3.org/2000/svg" width="500" height="500">""");
        for (int i = 0; i < 200; i++)
        {
            svg.Append(CultureInfo.InvariantCulture, $"""<rect x="{i}" y="0" width="1" height="1" fill="#000000"/>""");
        }
        svg.Append("</svg>");

        VectorImage image = Decoder.Decode(Encoding.UTF8.GetBytes(svg.ToString()), tiny);

        image.IsTruncated.ShouldBeTrue();
        image.Content.Count.ShouldBeLessThan(200);
        image.Diagnostics.ShouldContain(diagnostic => diagnostic.Code == "PL6001");
    }

    [Fact]
    public void AGzippedSvgIsRead()
    {
        // .svgz is what LibreOffice's own format list calls a gzipped SVG, and a package can
        // hold one under a plain image/svg+xml media type — so the bytes decide, not the label.
        byte[] plain = Encoding.UTF8.GetBytes(
            """<svg xmlns="http://www.w3.org/2000/svg" width="100" height="50"><rect width="20" height="10" fill="#ff0000"/></svg>""");

        using MemoryStream compressed = new();
        using (GZipStream gzip = new(compressed, CompressionLevel.Optimal, leaveOpen: true))
        {
            gzip.Write(plain);
        }

        byte[] bytes = compressed.ToArray();

        Decoder.CanDecode(bytes).ShouldBeTrue();

        VectorImage image = Decoder.Decode(bytes);
        image.IsEmpty.ShouldBeFalse();
        image.IntrinsicSize.Width.Emu.ShouldBe(100 * 9525);
    }

    [Fact]
    public void RubbishIsNotClaimedAndDoesNotThrow()
    {
        byte[] noise = [.. Enumerable.Range(0, 512).Select(i => (byte)(i * 37))];

        Decoder.CanDecode(noise).ShouldBeFalse();
        VectorImages.For(noise).ShouldBeNull();

        // And decoding it anyway is a diagnostic rather than an exception, because a picture
        // that will not read is a document to draw without it.
        Decoder.Decode(noise).IsEmpty.ShouldBeTrue();
    }

    [Fact]
    public void TruncatedXmlIsADiagnosticRatherThanAnException()
    {
        VectorImage image = Decode("""<svg xmlns="http://www.w3.org/2000/svg" width="100" height="50"><rect width="1" heig""");

        image.IsEmpty.ShouldBeTrue();
        image.Diagnostics.ShouldNotBeEmpty();
    }

    /// <summary>A four-by-four blue PNG, base64-encoded.</summary>
    private const string Png =
        "iVBORw0KGgoAAAANSUhEUgAAAAQAAAAECAIAAAAmkwkpAAAAD0lEQVR4nGNgaPiPQMRxAF7CF/FUdRSEAAAAAElFTkSuQmCC";

    /// <summary>
    /// A chain of groups, each holding <paramref name="fanOut"/> references to the one below.
    /// </summary>
    private static string UseBomb(int levels, int fanOut)
    {
        StringBuilder svg = new(
            """<svg xmlns="http://www.w3.org/2000/svg" width="10" height="10"><defs><g id="g0"><rect width="1" height="1"/></g>""");

        for (int level = 1; level <= levels; level++)
        {
            svg.Append(CultureInfo.InvariantCulture, $"""<g id="g{level}">""");
            for (int i = 0; i < fanOut; i++)
            {
                svg.Append(CultureInfo.InvariantCulture, $"""<use href="#g{level - 1}"/>""");
            }
            svg.Append("</g>");
        }

        svg.Append(CultureInfo.InvariantCulture, $"""</defs><use href="#g{levels}"/></svg>""");
        return svg.ToString();
    }

    private static VectorImage Decode(string svg) => Decoder.Decode(Encoding.UTF8.GetBytes(svg));
}
