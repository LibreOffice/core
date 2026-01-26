# CLAUDE.md - AI Assistant Guide for LibreOffice Core

This document provides guidance for AI assistants working with the LibreOffice Core codebase.

## Project Overview

LibreOffice is an open-source office suite maintained by The Document Foundation (TDF). This is the core repository containing ~200 modules implementing Writer, Calc, Impress/Draw, and supporting infrastructure.

**Key Links:**
- Documentation: https://docs.libreoffice.org/
- API Reference: https://api.libreoffice.org/
- Wiki: https://wiki.documentfoundation.org/Development/
- IRC: #libreoffice-dev on irc.libera.chat

## Build System

### Quick Start

```bash
# Configure (reads options from autogen.input if present)
./autogen.sh

# Build (use -jN for parallel builds)
make

# Run LibreOffice from build
bin/run soffice
```

### Configuration Options

Configuration is handled by `autogen.sh` which wraps autoconf. Options can be:
- Passed directly: `./autogen.sh --enable-debug`
- Stored in `autogen.input` file (one option per line)
- Used from distribution presets: `./autogen.sh --with-distro=LibreOfficeLinux`

Common options:
- `--enable-debug` - Enable debugging
- `--enable-dbgutil` - Enable debug utilities and compiler plugins
- `--enable-compiler-plugins` - Enable Clang plugins for code checking
- `--enable-odk` - Build the SDK
- `--with-parallelism=N` - Set build parallelism

Distribution configs are in `distro-configs/` directory.

### gbuild System

LibreOffice uses a custom build system called **gbuild** located in `solenv/gbuild/`. Key makefile types:
- `Library_*.mk` - Shared libraries
- `Executable_*.mk` - Executables
- `CppunitTest_*.mk` - Unit tests
- `Module_*.mk` - Module definitions
- `CustomTarget_*.mk` - Custom build targets

Each module has a `Makefile` and `Module_<name>.mk` defining its targets.

### Build Requirements

| Platform | Compiler | Runtime |
|----------|----------|---------|
| Linux | GCC 12+ or Clang 18+ | RHEL 9 / CentOS 9 |
| Windows | Visual Studio 2022 | Windows 10+ |
| macOS | Xcode 14.3+ | macOS 11+ |

- Java: JDK 17+
- Python: 3.11+

## Module Structure

### Core Application Modules

| Module | Description |
|--------|-------------|
| `sw/` | Writer (word processor) |
| `sc/` | Calc (spreadsheet) |
| `sd/` | Draw/Impress (graphics/presentation) |
| `desktop/` | Main application entry point (`soffice` binary) |
| `starmath/` | Math formula editor |
| `chart2/` | Chart component |

### Framework Modules

| Module | Description |
|--------|-------------|
| `sal/` | System Abstraction Layer - platform-independent types, strings, OS functions |
| `tools/` | Basic types (`Rectangle`, `Color`, etc.) |
| `vcl/` | Visual Class Library - widget toolkit and rendering |
| `sfx2/` | Legacy framework for document model, load/save, dispatch |
| `framework/` | UNO framework for toolbars, menus, status bars |
| `svx/` | Drawing model helpers, shared Draw/Impress code |

### Graphics Modules

| Module | Description |
|--------|-------------|
| `basegfx/` | Graphics algorithms and data types |
| `canvas/` | UNO canvas rendering with various backends |
| `drawinglayer/` | View code rendering drawable objects to primitives |
| `cppcanvas/` | C++ helpers for canvas |

### UNO Component System

| Module | Description |
|--------|-------------|
| `cppu/` | Core UNO runtime (C API and C++ wrappers) |
| `cppuhelper/` | C++ UNO component helpers |
| `bridges/` | UNO bridges (C++ ABI, Java, .NET) |
| `offapi/` | LibreOffice-specific IDL API definitions |
| `udkapi/` | Universal Development Kit API |
| `binaryurp/` | Binary UNO Remote Protocol |

### File Format Support

| Module | Description |
|--------|-------------|
| `oox/` | OOXML (Microsoft Office) format support |
| `xmloff/` | ODF XML format support |
| `filter/` | Document import/export filters |
| `writerperfect/` | Import filters for various formats |

### Other Key Modules

| Module | Description |
|--------|-------------|
| `compilerplugins/` | Clang compiler plugins for code checking |
| `libreofficekit/` | C/C++ API for document access without UNO |
| `test/` | Test framework and utilities |
| `uitest/` | Python UI testing framework |
| `external/` | 100+ external library dependencies |

Each module has a `README.md` with detailed documentation.

## Testing

### Test Commands

```bash
make unitcheck        # Fast unit tests
make slowcheck        # Slower integration tests
make subsequentcheck  # Tests requiring full build
make uicheck          # UI tests (Linux only by default)
make check            # All of the above
```

### Test Structure

Tests are in `qa/` subdirectories within each module:
- `qa/unit/` - Unit tests
- `qa/cppunit/` - CppUnit-based tests
- `qa/uitest/` - Python UI tests

### Running Specific Tests

```bash
# Run tests for a specific module
make -C sw check

# Run a specific test
make CppunitTest_sw_uiwriter
```

### Smoke Testing

The `smoketest/` module provides basic sanity tests that connect via UNO socket.

## Code Conventions

### Formatting

**C/C++ Style** (from `.clang-format`):
- 4-space indentation, no tabs
- 100 character line limit
- Allman brace style (braces on own line)
- Pointer binds to type: `Type* ptr`

**EditorConfig** (`.editorconfig`):
- UTF-8 charset, LF line endings
- 4-space indent for source files
- Tab indent for Makefiles only
- Trim trailing whitespace

### Include Directives

```cpp
// Use "" only for files next to the including file
#include "local_header.hxx"

// Use <> for everything else
#include <sal/types.h>
#include <com/sun/star/uno/Reference.hxx>
```

This is enforced by `loplugin:includeform`.

### Naming Conventions

- Classes: `PascalCase`
- Member variables: `m_` prefix (e.g., `m_pDocument`)
- Pointers often use `p` prefix
- UNO interfaces: `X` prefix (e.g., `XComponent`)

### Pre-commit Checks

The `.git-hooks/pre-commit` script checks for:
- Trailing whitespace
- DOS line endings
- Tab characters in source files
- Merge conflict markers
- `SAL_DEBUG` temporary debug statements
- German comments

Install hooks: `cd .git/hooks && ln -s ../../.git-hooks/* ./`

## Compiler Plugins

LibreOffice uses Clang compiler plugins for additional code checking and automated refactoring.

### Enabling

```bash
./autogen.sh --enable-compiler-plugins  # Explicit
./autogen.sh --enable-dbgutil           # Auto-enables if Clang headers found
```

Requires Clang 18+ with development headers.

### Usage

Plugins run automatically during compilation. Warnings/errors are prefixed with `[loplugin]`.

For rewriting tools:
```bash
make COMPILER_PLUGIN_TOOL=<tool_name> FORCE_COMPILE=all UPDATE_FILES=all
```

### Key Plugins

Located in `compilerplugins/clang/`:
- `includeform` - Check #include style
- `writeonlyvars` - Detect write-only variables
- `unusedmethods` - Find unused methods

## Key Tools and Scripts

### Build Tools

| Tool | Purpose |
|------|---------|
| `autogen.sh` | Configure the build |
| `bin/run` | Run executables from workdir |
| `solenv/bin/` | Build system utilities |
| `Makefile.fetch` | Download external dependencies |

### Development Scripts

| Script | Purpose |
|--------|---------|
| `bin/find-german-comments` | Find German language comments |
| `solenv/clang-format/` | Code formatting utilities |
| `logerrit` | Gerrit integration script |

### Debugging

GDB helpers are in `solenv/gdb/` for debugging UNO types and LibreOffice structures.

## Common Development Tasks

### Adding a New File

1. Create the source file in the appropriate module directory
2. Add to the module's `Library_*.mk` or equivalent makefile
3. Follow existing patterns in the module

### Modifying UI

UI definitions are in `uiconfig/` subdirectories as `.ui` files (GtkBuilder format). The framework module loads these via XML descriptions.

### Working with UNO

- IDL definitions in `offapi/` and `udkapi/`
- Use `cppuhelper` utilities for component implementation
- See `odk/examples/` for SDK examples

### Cross-Platform Considerations

- Use `sal/` types for platform independence
- VCL abstracts platform-specific rendering
- Platform code isolated in `vcl/*/` backend directories

## Architecture Notes

### Document Models

- **Writer**: `SwDoc` is the central document class
- **Calc**: `ScDocument` with column-oriented storage
- **Draw/Impress**: `SdrModel` and `SdrView`

### Rendering Pipeline

1. Document model generates `Primitive2D` objects
2. `drawinglayer` processors render primitives
3. VCL backends output to screen/PDF/etc.

### Framework Layers

- **VCL**: Widget toolkit abstraction
- **SFX2**: Legacy document framework (still used by main apps)
- **UNO Framework**: Modern component-based framework for UI chrome

## Important Patterns

### Resource Management

- Use smart pointers (`std::unique_ptr`, `rtl::Reference`)
- VCL windows managed via `VclPtr<>`
- UNO references via `css::uno::Reference<>`

### Error Handling

- Use `SAL_WARN`, `SAL_INFO` for logging
- `SAL_DEBUG` for temporary debug output (blocked by pre-commit)
- Assertions via `assert()` or `DBG_ASSERT`

### Memory

- Prefer stack allocation when possible
- Use LibreOffice allocators for large data

## Anti-Patterns to Avoid

- Don't use raw `new`/`delete` for VCL objects (use `VclPtr`)
- Don't mix `""` and `<>` include styles incorrectly
- Don't add tabs to source files
- Don't commit `SAL_DEBUG` statements
- Don't ignore compiler plugin warnings

## Getting Help

- Module-specific docs: `<module>/README.md`
- Mailing list: libreoffice@lists.freedesktop.org
- IRC: #libreoffice-dev on irc.libera.chat
- Wiki: https://wiki.documentfoundation.org/Development/

## Quick Reference

```bash
# Full build
./autogen.sh && make

# Debug build
./autogen.sh --enable-debug --enable-dbgutil && make

# Run after build
bin/run soffice

# Run tests
make check

# Build single module
make sw

# Format code
make COMPILER_PLUGIN_TOOL=includeform FORCE_COMPILE=all

# Fetch dependencies
make fetch
```
