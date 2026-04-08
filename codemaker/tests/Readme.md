# Codemaker Test Suite

This directory contains the integration test suite for the UNO code generators `cppumaker` and `pythonmaker`. The suite ensures that:

- `cppumaker` correctly generates C++ headers (`.hpp` and `.hdl`), and
- `pythonmaker` correctly generates Python stub files (`.pyi`)

from the same UNO IDL definitions. It verifies the syntactic correctness of the output and protects against future regressions.

The tests are automatically executed as part of the `make check` command for the `codemaker` module. They can also be run on their own with:

```bash
make codemaker.check
```

## Testing Philosophy

The test suite follows a "Golden File" testing methodology. This is a standard and robust approach for testing compilers and code generators. The process is as follows:

1.  **Source IDL:** A single comprehensive test IDL file (`tests.idl`) serves as the source of truth for all UNO constructs that the generators are expected to handle. Both tools are fed the same file.
2.  **Generate Output:** During the test run, this IDL is compiled into a temporary `.rdb` file with `unoidl-write`, and each tool (`cppumaker`, `pythonmaker`) is executed to generate its output in its own temporary directory.
3.  **Compare Against "Golden" Files:** The newly generated output is recursively compared against a set of manually verified, correct "golden" files:
    - `expected_cppumaker_stubs/` for `cppumaker`
    - `expected_pythonmaker_stubs/` for `pythonmaker`
4.  **Pass/Fail:** A test passes only if the generated output is an exact match to the golden files. Any difference (missing files, extra files, or content mismatches) causes the test to fail.

This approach ensures that any change to a generator that alters its output is immediately detected.

## Directory Structure

- `tests.idl`: The master IDL file containing all test cases. This includes enums, constants, typedefs, structs (plain, inherited, and polymorphic), exceptions, interfaces, services, and singletons. It also includes edge cases like the use of keywords as identifiers. All types live in the tool-neutral module `test::codemaker::codemakertests`, so the generated paths and namespaces (e.g. `test/codemaker/codemakertests/...`) are the same for both tools.
- `expected_cppumaker_stubs/`: The "golden" `.hpp` and `.hdl` file structure that `cppumaker` is expected to generate from `tests.idl`. This is the reference standard for correctness of `cppumaker`.
- `expected_pythonmaker_stubs/`: The "golden" `.pyi` file structure that `pythonmaker` is expected to generate from `tests.idl`. This is the reference standard for correctness of `pythonmaker`.
- `codemakertests.py`: The Python script that orchestrates the tests. It is executed by the build system and, for each tool, compiles the IDL, runs the tool, and compares the output directory with the matching golden directory. It contains one test class per tool (`TestCppuMaker`, `TestPythonMaker`) sharing a common implementation.
- `README.md`: This file.

The gbuild makefile that integrates the script into `make check` is `codemaker/Test_codemaker.mk`, registered in `codemaker/Module_codemaker.mk`. It depends on both the `cppumaker` and `pythonmaker` executables.

---

## How to Modify or Extend the Test Suite

Future developers may need to modify these tests when fixing a bug or adding a new feature to either tool. Here is the standard workflow.

### Regenerating the Golden Files

Both golden directories are regenerated from `tests.idl` with the freshly built tools. Run the following from `instdir/program` (the tools need the libraries located there):

```bash
mkdir -p ../../workdir/golden_tmp

# 1. Compile the shared IDL to an .rdb
../sdk/bin/unoidl-write.exe ../../workdir/UnoApiTarget/udkapi.rdb ../../codemaker/tests/tests.idl ../../workdir/golden_tmp/tests.rdb

# 2. Clear the old golden files
rm -rf ../../codemaker/tests/expected_cppumaker_stubs ../../codemaker/tests/expected_pythonmaker_stubs
mkdir -p ../../codemaker/tests/expected_cppumaker_stubs ../../codemaker/tests/expected_pythonmaker_stubs

# 3. Generate the stubs straight into the golden directories
../sdk/bin/cppumaker.exe -O ../../codemaker/tests/expected_cppumaker_stubs ../../workdir/golden_tmp/tests.rdb -X ../../workdir/UnoApiTarget/udkapi.rdb
../sdk/bin/pythonmaker.exe -O ../../codemaker/tests/expected_pythonmaker_stubs ../../workdir/golden_tmp/tests.rdb -X ../../workdir/UnoApiTarget/udkapi.rdb

# 4. Remove leftovers (the test ignores *.tmp files of pythonmaker)
find ../../codemaker/tests/expected_pythonmaker_stubs -name '*.tmp' -delete
rm -rf ../../workdir/golden_tmp
```

(On Linux/macOS, drop the `.exe` suffix; the SDK tools are in `instdir/sdk/bin`, or in the `LibreOffice*_SDK/bin` directory of the installation.) Only one of the two tools has to be regenerated if only one is affected; the other directory is left untouched. Always review the result with `git diff` before committing.

### Scenario 1: Fixing a Bug in a Generator

If you have fixed a bug that was causing a tool to generate incorrect output, the corresponding test should now fail because the new, correct output will not match the old, incorrect golden files.

1.  **Verify the Fix:** After fixing the C++ code of the tool, run `make codemaker.check` to confirm that the test fails as expected. The error output lists the differing files and shows the first difference.
2.  **Regenerate the Golden Files:** Follow the steps in "Regenerating the Golden Files" above.
3.  **Review the Changes:** Use `git diff` to confirm that only the intended differences appear in the golden files.
4.  **Validate:**
    - For `cppumaker`: ensure the new generated headers are syntactically correct and type-safe (often verified simply by checking that a full LibreOffice build succeeds with your new `cppumaker`).
    - For `pythonmaker`: run `mypy --strict codemaker/tests/expected_pythonmaker_stubs/` to ensure the new golden files are syntactically correct and type-safe.
5.  **Commit:** Commit the changes to the tool's C++ source code **along with** the updated golden files in your patch. The commit message should explain that the golden files were updated to reflect the bug fix.

### Scenario 2: Adding a New Feature to a Generator

If you add support for a new IDL feature (e.g., a new type or an annotation), you should add a test case for it.

1.  **Add a Test Case to the IDL:**
    - Open `codemaker/tests/tests.idl`.
    - Add a new, simple example of the feature you've implemented. For example, add a new `interface` with a specific attribute you are now supporting.
2.  **Generate and Verify the New Golden Files:**
    - Since you've added a new type, new output files will be generated. Regenerate the golden files as described above. Note that the IDL is shared, so a change to `tests.idl` may also change the output of the _other_ tool; review the golden files of both tools.
    - Locate the newly generated files for your new test case.
    - **Manually inspect these new files** to ensure they are 100% correct (valid C++ for `cppumaker`; run `mypy --strict` on them for `pythonmaker`).
3.  **Run the Full Test Suite:** Run `make codemaker.check`. The tests should now pass, as the newly generated output will match your newly added golden files.
4.  **Commit:** Commit the changes to the tool's source code, the updated `tests.idl`, and the new golden files together in your patch.
