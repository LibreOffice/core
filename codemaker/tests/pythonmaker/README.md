# Pythonmaker Test Suite

This directory contains the integration test suite for the `pythonmaker` tool. The purpose of this suite is to ensure that `pythonmaker` correctly generates Python stub files (`.pyi`) from UNO IDL definitions. It verifies the syntactic correctness of the output and protects against future regressions.

The test is automatically executed as part of the `make check` command for the `codemaker` module.

## Testing Philosophy

The test suite follows a "Golden File" testing methodology. This is a standard and robust approach for testing compilers and code generators. The process is as follows:

1.  **Source IDL:** A comprehensive test IDL file (`idl/pythontypes.idl`) serves as the single source of truth for all UNO constructs that `pythonmaker` is expected to handle.
2.  **Generate Output:** During the test run, this IDL is compiled into an `.rdb` file, and `pythonmaker` is executed to generate a complete set of `.pyi` stubs in a temporary directory.
3.  **Compare Against "Golden" Files:** The newly generated stubs are recursively compared against a set of manually verified, correct "golden" stub files located in the `expected_pyi_stubs/` directory.
4.  **Pass/Fail:** The test passes only if the generated output is an exact match to the golden files. Any difference—missing files, extra files, or content mismatches—will cause the test to fail.

This approach ensures that any change to `pythonmaker` that alters its output is immediately detected.

## Directory Structure

-   `idl/pythontypes.idl`: The master IDL file containing all test cases. This includes enums, constants, typedefs, structs (plain, inherited, and polymorphic), exceptions, interfaces, services, and singletons. It also includes edge cases like the use of keywords as identifiers.
-   `expected_pyi_stubs/`: Contains the "golden" `.pyi` file structure that `pythonmaker` is expected to generate from `pythontypes.idl`. This is the reference standard for correctness.
-   `test_pythonmaker.py`: The Python script that orchestrates the entire test. It is executed by the build system and is responsible for compiling the IDL, running `pythonmaker`, and performing the directory comparison.
-   `makefile.mk`: The gbuild makefile that integrates the Python test script into the LibreOffice `make check` process.

---

## How to Modify or Extend the Test Suite

Future developers may need to modify these tests when fixing a bug or adding a new feature to `pythonmaker`. Here is the standard workflow.

### Scenario 1: Fixing a Bug in `pythonmaker`

If you have fixed a bug that was causing `pythonmaker` to generate incorrect `.pyi` files, the test suite should now fail because the new, correct output will not match the old, incorrect golden files.

**To update the tests:**

1.  **Verify the Fix:** After fixing the C++ code in `pythonmaker`, run the test to confirm that it fails as expected. The `diff` error will show you the difference between the new output and the old golden files.
2.  **Regenerate the Golden Files:**
    a. First, delete the old golden files to ensure a clean slate.
       ```bash
       rm -rf codemaker/tests/pythonmaker/expected_pyi_stubs/*
       ```
    b. Run your updated `pythonmaker` manually to generate the new, correct stubs into a temporary directory. You can find the exact command to run from the test script's log output during a `make check` run. A typical command would be:
       ```bash
       # Run from .../libreoffice/instdir/program/
       ./pathToPythonmaker.exe -O /path/to/new_golden_output /path/to/test.rdb
       ```
3.  **Replace the Golden Files:** Copy the entire generated output into the `expected_pyi_stubs/` directory.
    ```bash
    cp -r /path/to/new_golden_output/* codemaker/tests/pythonmaker/expected_pyi_stubs/
    ```
4.  **Validate:** Run `mypy --strict codemaker/tests/pythonmaker/expected_pyi_stubs/` to ensure the new golden files are syntactically correct and type-safe.
5.  **Commit:** Commit the changes to `pythonmaker`'s C++ code **along with** the updated golden files in your patch. The commit message should explain that the golden files were updated to reflect the bug fix.

### Scenario 2: Adding a New Feature to `pythonmaker`

If you add support for a new IDL feature (e.g., a new type or an annotation), you should add a test case for it.

1.  **Add a Test Case to the IDL:**
    *   Open `codemaker/tests/pythonmaker/idl/pythontypes.idl`.
    *   Add a new, simple example of the feature you've implemented. For example, add a new `interface` with a specific attribute you are now supporting.
2.  **Generate and Verify the New Golden File:**
    *   Since you've added a new type, a new `.pyi` file will be generated. Run `pythonmaker` manually as described above.
    *   Locate the newly generated `.pyi` file for your new test case.
    *   **Manually inspect this new file** to ensure it is 100% correct.
    *   Run `mypy --strict` on this individual file.
    *   Once verified, place this new golden file in the correct subdirectory within `expected_pyi_stubs/`.
3.  **Run the Full Test Suite:** Run `make check` for the module. The test should now pass, as the newly generated output will match your newly added golden file.
4.  **Commit:** Commit the changes to `pythonmaker`'s C++ code, the updated `pythontypes.idl`, and the new golden `.pyi` file(s) together in your patch.

By following this workflow, the test suite remains a reliable and comprehensive measure of `pythonmaker`'s correctness.
