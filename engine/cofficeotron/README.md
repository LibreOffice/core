# cofficeotron

A validator for office documents: ODF packages (ODT, ODS, ODP) and
OOXML packages (DOCX, XLSX, PPTX). This is a C++ port of the Java
Office-o-tron (officeotron) validator; the export-validation test harness runs it over
exported documents.

## Building

cofficeotron is a gbuild module. configure adds it to the build (the
COFFICEOTRON build type) only when the --enable-cofficeotron switch is
passed; without it, export validation keeps using the Java officeotron
from the PATH or the downloaded jar. The binary lands in
workdir/LinkTarget/Executable/. The zip reading code of the package
module is linked in as library objects, and the expat, rnv, zlib,
zstd and argon2 dependencies are the engine's own externals. The
module also builds by hand, whatever the build type:

    cd cofficeotron && make            # build the module
    cd cofficeotron && make slowcheck  # run the tests

When configure selects the native validator, the export validation
harness runs it through bin/officeotron.sh. In a build without the
switch that wrapper runs the downloaded Java officeotron jar instead.

## Usage

    cofficeotron [--errors-only] [--check-ids] [--force-is] [--schema-dir DIR] file1 ...

The tool also answers `--help` and `--version`.

- `--errors-only` suppresses the INFO commentary, leaving warnings and
  errors.
- `--check-ids` cross-checks ID and IDREF attribute values in ODF
  documents.
- `--force-is` validates ODF documents against the ISO/IEC 26300
  (ODF 1.0) schema regardless of their claimed version.
- `--schema-dir DIR` points at the schema directory. Without it the tool
  checks the `COFFICEOTRON_SCHEMA_DIR` environment variable, then
  `etc/schema` next to the executable, one level above it, or under the
  current directory.

The tool prints a validation narrative to standard output. The summary
lines are stable interfaces that other tooling parses:

    Grand total count of validity errors: N        (ODF)
    Grand total of errors in submitted package: N  (OOXML, only when N > 0)

The exit code is 0 when validation ran (whether or not the documents
were valid) and 2 for usage or input/output problems.

## Schemas

All schemas are RELAX NG compact syntax, the form the RNV validator
reads. The build assembles them into one directory,
workdir/CustomTarget/cofficeotron/schema, from two sources:

- `etc/schema/odf/` in the source tree - the OASIS ODF schemas,
  converted from the published XML syntax with trang.
- The ECMA-376 schemas for OOXML - the RELAX NG annexes of the 5th
  edition standard archives (part 4 transitional markup, part 2
  packaging), which external/ecma376 downloads, unpacks and patches.
  They are not kept in the source tree because Ecma International
  publishes them under its own copyright terms, not under an open
  source license. Two patches carry over relaxations from the Java
  officeotron 0.8.8 schema set: xml:space on spreadsheet text elements
  and percent-mille bullet size values, both of which real-world
  documents contain.

## Behaviour differences from the Java officeotron

The report structure, the summary lines, the indentation, and the error
capping (10 printed errors per document part) match the Java tool. The
differences:

- Validation error wording comes from the RNV RELAX NG validator
  instead of jing and Xerces, so the text and count of messages on
  invalid content can differ. Valid content is expected to
  stay valid; divergences there are bugs worth reporting.
- The servlet (web) mode was not ported; the tool is CLI only.
- The ODF schemas are bundled. The Java tool downloaded them from
  oasis-open.org at startup and knew versions 1.0 to 1.2 only; this tool
  works offline and also covers 1.3 and 1.4. Version 1.2 uses the final
  OASIS Standard schema where the Java tool used the csprd03 draft.
- `--check-ids` and `--force-is` are new as command line flags (the Java
  CLI had no way to set them).
- Line numbers in error locations print without thousands separators,
  and the column is always 0: errors carry the line of the nearest
  enclosing element.
- A file that is neither an ODF nor an OOXML package produces a FATAL
  comment instead of a crash.

## Tests

The tests run in the slowcheck path, so the top-level `make check`
includes them when the module is in the build:

- `CppunitTest_cofficeotron` - ports of the Java JUnit suites: the MCE
  filter tests (filtering output and MCE misuse error counts), the OPC
  package spidering expectations for `etc/test-data/torture.pptx`, the
  zip extraction checks, and the whole-session error count for
  torture.pptx.
- `CustomTarget_cofficeotron_e2e` - runs the binary over
  `etc/test-data` and compares the full output against
  `tests/expected/`. The expected files embed RNV's message wording;
  after a schema or validator update regenerate them by running
  `tests/run-e2e.sh --regen` with the `COFFICEOTRON` environment
  variable naming the built binary and `COFFICEOTRON_SCHEMA_DIR` the
  assembled schema directory, and review the diff.

## License

MPL 2.0 - the code was ported from the MPL 1.1 Java officeotron sources,
and the license upgraded at the same time.
