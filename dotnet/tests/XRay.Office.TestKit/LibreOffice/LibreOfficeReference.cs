using System.Diagnostics;
using System.Globalization;
using System.Text;

namespace XRay.Office.TestKit.LibreOffice;

/// <summary>
/// Which LibreOffice the fidelity harness measures against, and whether it is fit to be the
/// oracle at all.
/// </summary>
/// <remarks>
/// <para>
/// <strong>The reference is 26.2.4.2, and it is named here rather than taken from
/// <c>PATH</c>.</strong> <c>/usr/bin/soffice</c> in this container is 24.2.7.2, and the two
/// versions do not merely differ in polish — they resolve fonts by different rules.
/// <c>FontConfigManager::Substitute</c> appends <c>"serif"</c> as a second <c>FC_FAMILY</c> for
/// <c>FAMILY_ROMAN</c> and <c>"sans"</c> for <c>FAMILY_SWISS</c>
/// (<c>vcl/unx/generic/font/fontconfig.cxx</c>:1075-1088) and <em>that switch does not exist in
/// 24.2</em>, so on 24.2 the family name decides and on 26.2 a declared class beats it. Over 24
/// one-family probe documents this tree matches 26.2 on 24 of 24 and 24.2 on 7 of 24
/// (<c>probes/font-fallback/run.sh</c>). Measuring against whichever <c>soffice</c> happened to be
/// first on <c>PATH</c> is how a round was dispatched to "fix" 119 correct documents.
/// </para>
/// <para>
/// <c>XRAY_OFFICE_SOFFICE</c> overrides the choice, so going back to 24.2 for a comparison is one
/// environment variable and is visible in the run that used it. <see cref="Describe"/> is what a
/// failing run should print.
/// </para>
/// <para>
/// <strong>And the reference is refused rather than trusted, because the way it goes wrong is
/// silent.</strong> A TDF tarball reads fonts out of its own <c>share/fonts/truetype</c> before
/// the system's, and it ships two kinds of trap:
/// </para>
/// <list type="number">
///   <item><description>
///     <b>The Latin Noto.</b> <c>NotoSans-*</c> and <c>NotoSerif-*</c> duplicate <em>nothing</em>
///     installed, so the documented <c>mv</c> for the duplicates does not catch them, and they
///     become fontconfig's answer for every family the system lacks. Every comparison on a
///     document naming an uninstalled family is then void, and it looks like a coherent result.
///     Caught by rendering a probe and reading the face out of it, which is the only method that
///     sees a bundled font at all — <c>fc-match</c> answers for the system set and knows nothing
///     about what a bundled application resolves.
///   </description></item>
///   <item><description>
///     <b>The metric-compatible duplicates.</b> The bundle's Carlito, Caladea, Liberation and
///     DejaVu are <em>different builds</em> from the system's — Caladea-Regular is 58 964 bytes
///     bundled against 81 600 installed — so both sides draw a family of the same name with
///     different advances and every text metric diverges. Invisible to a face-name check,
///     because the name is identical; caught by basename against the system font tree, which
///     flags exactly the files that shadow an installed one and leaves
///     <c>DejaVuSansCondensed</c> and friends alone.
///   </description></item>
/// </list>
/// <para>
/// The script-specific Noto — <c>NotoSansArabic</c>, <c>NotoSerifHebrew</c> and the rest — must
/// stay. It carries coverage the system genuinely lacks, and removing it changes what an Arabic
/// or Hebrew document can draw at all.
/// </para>
/// </remarks>
public static class LibreOfficeReference
{
    /// <summary>The reference this tree targets when nothing overrides it.</summary>
    public const string PinnedPath = "/opt/libreoffice26.2/program/soffice";

    /// <summary>The environment variable that overrides <see cref="Path"/>.</summary>
    public const string OverrideVariable = "XRAY_OFFICE_SOFFICE";

    /// <summary>The <c>soffice</c> the harness drives.</summary>
    /// <remarks>
    /// <c>XRAY_OFFICE_SOFFICE</c> wins; then the pinned 26.2, when it is there; then whatever is on
    /// <c>PATH</c>, so a machine with one LibreOffice still works and says which one it used.
    /// </remarks>
    public static string Path => Resolved.Value;

    private static readonly Lazy<string> Resolved = new(() =>
    {
        if (Environment.GetEnvironmentVariable(OverrideVariable) is { Length: > 0 } chosen)
            return chosen;

        return File.Exists(PinnedPath) ? PinnedPath : "soffice";
    });

    /// <summary>
    /// True when the reference exists, converts, and its font environment is not shadowed.
    /// </summary>
    public static bool IsUsable => Check.Value.Usable;

    /// <summary>Why the reference cannot be used, or an empty string when it can.</summary>
    public static string Reason => Check.Value.Reason;

    /// <summary>A one-line description of the reference in force, for a failure message.</summary>
    public static string Describe()
        => string.Create(
            CultureInfo.InvariantCulture,
            $"reference {Path} ({Check.Value.Version}); unfiled family resolves to "
            + $"{(Check.Value.UnfiledFace.Length == 0 ? "nothing" : Check.Value.UnfiledFace)}");

    private static readonly Lazy<ReferenceCheck> Check = new(Inspect);

    private readonly record struct ReferenceCheck(
        bool Usable, string Reason, string Version, string UnfiledFace);

    private static ReferenceCheck Inspect()
    {
        string binary = Path;

        using LibreOfficeRunner runner = new(binary);
        string version = runner.GetVersion();
        if (version.Length == 0)
        {
            return new ReferenceCheck(
                false, $"LibreOffice is not installed or will not start at '{binary}'.", "", "");
        }

        if (Shadowed(binary) is { Length: > 0 } shadowed)
            return new ReferenceCheck(false, shadowed, version, "");

        string face = UnfiledFamilyFace(runner);
        if (face.Length == 0)
        {
            return new ReferenceCheck(
                false,
                $"'{binary}' produced no readable font list for the fallback probe, so the "
                + "reference's font resolution could not be established.",
                version,
                "");
        }

        if (!face.Contains("DejaVu", StringComparison.OrdinalIgnoreCase))
        {
            return new ReferenceCheck(
                false,
                $"'{binary}' draws an uninstalled family in '{face}' rather than a DejaVu face. "
                + "A TDF tarball reads fonts out of its own share/fonts/truetype, and its Latin "
                + "Noto (NotoSans-*, NotoSerif-*) duplicates nothing installed, so the documented "
                + "mv for the metric-compatible duplicates leaves it in place and it answers for "
                + "every family the system lacks. Every comparison here would then be void. Fix:\n"
                + "  D=/opt/libreoffice26.2/share/fonts/truetype\n"
                + "  mkdir -p $D/.noto-aside && mv $D/Noto{Sans,Serif}-*.ttf $D/.noto-aside/\n"
                + "Leave NotoSansArabic, NotoSerifHebrew and the other script-specific faces: they "
                + "carry coverage the system does not have.",
                version,
                face);
        }

        return new ReferenceCheck(true, string.Empty, version, face);
    }

    /// <summary>
    /// Bundled faces whose basename also exists in the system font tree, which are the ones that
    /// shadow an installed build of the same family.
    /// </summary>
    /// <remarks>
    /// Basename rather than a hard-coded family list, so a tarball that starts bundling something
    /// new is caught without this being edited, and so the faces that duplicate nothing —
    /// <c>DejaVuSansCondensed</c>, <c>LiberationSansNarrow</c>, <c>DejaVuMathTeXGyre</c> — are not
    /// flagged. Returns an empty string when there is no bundle to check, which is every distro
    /// install.
    /// </remarks>
    private static string Shadowed(string binary)
    {
        string? program = System.IO.Path.GetDirectoryName(System.IO.Path.GetFullPath(binary));
        string? install = program is null ? null : System.IO.Path.GetDirectoryName(program);
        if (install is null) return string.Empty;

        string bundle = System.IO.Path.Combine(install, "share", "fonts");
        if (!Directory.Exists(bundle)) return string.Empty;

        Dictionary<string, string> installed = new(StringComparer.OrdinalIgnoreCase);
        foreach (string root in new[] { "/usr/share/fonts", "/usr/local/share/fonts" })
        {
            if (!Directory.Exists(root)) continue;
            foreach (string file in Directory.EnumerateFiles(root, "*", SearchOption.AllDirectories))
                installed[System.IO.Path.GetFileName(file)] = file;
        }
        if (installed.Count == 0) return string.Empty;

        List<string> clashes = [];
        foreach (string file in Directory.EnumerateFiles(bundle, "*", SearchOption.AllDirectories))
        {
            // A directory the recipe has already moved things into is where they are supposed to
            // be. fontconfig skips a dot-directory, and so does this.
            if (file.Contains("/.", StringComparison.Ordinal)) continue;

            string name = System.IO.Path.GetFileName(file);
            if (!installed.TryGetValue(name, out string? system)) continue;

            // The same file under two paths is not a shadow. Only a *different build* of the same
            // family is, because then the two sides measure different advances while reporting the
            // same family name — which is the whole reason a name-level check cannot find this.
            if (SameBytes(file, system)) continue;

            clashes.Add(name);
        }
        if (clashes.Count == 0) return string.Empty;

        clashes.Sort(StringComparer.Ordinal);
        StringBuilder message = new();
        message.Append(CultureInfo.InvariantCulture, $"'{binary}' bundles {clashes.Count} font ");
        message.Append("file(s) that shadow an installed build of the same family, so the two ");
        message.Append("sides would measure different files and every advance width would ");
        message.Append("diverge: ");
        message.Append(string.Join(", ", clashes.Take(8)));
        if (clashes.Count > 8) message.Append(CultureInfo.InvariantCulture, $" and {clashes.Count - 8} more");
        message.Append(". Fix:\n  D=");
        message.Append(System.IO.Path.Combine(install, "share", "fonts", "truetype"));
        message.Append("\n  mkdir -p $D/.duplicates-aside && ");
        message.Append("mv $D/{Carlito,Caladea,Liberation,DejaVu}*.ttf $D/.duplicates-aside/");
        return message.ToString();
    }

    /// <summary>Whether two files hold the same bytes.</summary>
    /// <remarks>
    /// Length first, because it separates almost every pair for the cost of two stats, and the
    /// content read only settles the rest. An unreadable file is treated as different: refusing to
    /// compare is the safe direction.
    /// </remarks>
    private static bool SameBytes(string left, string right)
    {
        try
        {
            FileInfo a = new(left), b = new(right);
            if (a.Length != b.Length) return false;
            return File.ReadAllBytes(left).AsSpan().SequenceEqual(File.ReadAllBytes(right));
        }
        catch (Exception exception) when (exception is IOException or UnauthorizedAccessException)
        {
            return false;
        }
    }

    /// <summary>
    /// The face the reference draws for a family nothing on the machine has.
    /// </summary>
    /// <remarks>
    /// A flat ODF file, because it is one XML file with no packaging, and the question — which
    /// family answers for a name nobody has — is the same through every filter. The face is read
    /// out of the produced PDF rather than asked of <c>fc-match</c>, which is the whole point:
    /// <c>fc-match</c> answers for the system font set and cannot see a bundled font.
    /// </remarks>
    private static string UnfiledFamilyFace(LibreOfficeRunner runner)
    {
        string work = System.IO.Path.Combine(
            System.IO.Path.GetTempPath(),
            "xray-office-ref-" + Guid.NewGuid().ToString("N")[..12]);

        try
        {
            Directory.CreateDirectory(work);
            string source = System.IO.Path.Combine(work, "unfiled-family-probe.fodt");
            File.WriteAllText(source, FallbackProbe);

            string pdf = System.IO.Path.Combine(work, "out", "unfiled-family-probe.pdf");
            Directory.CreateDirectory(System.IO.Path.Combine(work, "out"));
            runner.ConvertToPdf(source, System.IO.Path.Combine(work, "out"));
            if (!File.Exists(pdf)) return string.Empty;

            return FirstFace(pdf);
        }
        catch (Exception exception) when (exception is not OutOfMemoryException)
        {
            return string.Empty;
        }
        finally
        {
            try { if (Directory.Exists(work)) Directory.Delete(work, recursive: true); }
            catch (IOException) { }
            catch (UnauthorizedAccessException) { }
        }
    }

    /// <summary>The first face named by <c>pdffonts</c>, with any subset tag stripped.</summary>
    /// <remarks>
    /// A one-paragraph document draws in one face, so the first row is the answer. The
    /// <c>ABCDEF+</c> prefix is assigned per file and says nothing about which family was
    /// resolved, so it comes off before the name is compared.
    /// </remarks>
    private static string FirstFace(string pdf)
    {
        ProcessStartInfo start = new("pdffonts")
        {
            RedirectStandardOutput = true,
            RedirectStandardError = true,
        };
        start.ArgumentList.Add(pdf);

        using Process? process = Process.Start(start);
        if (process is null) return string.Empty;

        string output = process.StandardOutput.ReadToEnd();
        process.WaitForExit((int)LibreOfficeRunner.Timeout.TotalMilliseconds);
        if (process.ExitCode != 0) return string.Empty;

        // Two header lines, then one row per face; the name is the first column.
        foreach (string line in output.Split('\n').Skip(2))
        {
            string[] fields = line.Split(' ', StringSplitOptions.RemoveEmptyEntries);
            if (fields.Length == 0) continue;

            string name = fields[0];
            int plus = name.IndexOf('+', StringComparison.Ordinal);
            return plus == 6 ? name[7..] : name;
        }
        return string.Empty;
    }

    /// <summary>
    /// One paragraph in a family no machine has, which is what makes the answer diagnostic.
    /// </summary>
    /// <remarks>
    /// <c>Zzzz Nonexistent Family</c> rather than a real absent name such as <c>Verdana</c>: a
    /// real name can be filed under a generic by <c>45-latin.conf</c> or aliased by
    /// <c>30-metric-aliases.conf</c>, and then the answer says something about the configuration
    /// instead of about the bundle.
    /// </remarks>
    private const string FallbackProbe = """
        <?xml version="1.0" encoding="UTF-8"?>
        <office:document
            xmlns:office="urn:oasis:names:tc:opendocument:xmlns:office:1.0"
            xmlns:style="urn:oasis:names:tc:opendocument:xmlns:style:1.0"
            xmlns:text="urn:oasis:names:tc:opendocument:xmlns:text:1.0"
            xmlns:svg="urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0"
            office:version="1.3" office:mimetype="application/vnd.oasis.opendocument.text">
          <office:font-face-decls>
            <style:font-face style:name="probe" svg:font-family="'Zzzz Nonexistent Family'"/>
          </office:font-face-decls>
          <office:automatic-styles>
            <style:style style:name="P1" style:family="paragraph">
              <style:text-properties style:font-name="probe"/>
            </style:style>
          </office:automatic-styles>
          <office:body><office:text>
            <text:p text:style-name="P1">Handgloves 12345</text:p>
          </office:text></office:body>
        </office:document>
        """;
}
