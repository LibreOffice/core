#!/usr/bin/env python3
"""
Verification script to check if Google Drive integration was built successfully
"""

import os
import sys
from pathlib import Path

def check_build_status():
    """Check if Google Drive integration built successfully"""

    core_path = Path(__file__).parent.absolute()
    print(f"🔍 Checking Google Drive build status in: {core_path}")
    print("=" * 60)

    results = {
        "source_files": True,
        "library_built": False,
        "component_file": False,
        "installation": False
    }

    # 1. Check source files exist
    print("1. Checking source files...")
    source_files = [
        "ucb/source/ucp/gdrive/GoogleDriveApiClient.cxx",
        "ucb/source/ucp/gdrive/gdrive_provider.cxx",
        "ucb/source/ucp/gdrive/gdrive_content.cxx",
        "ucb/source/ucp/gdrive/gdrive_json.cxx"
    ]

    for file_path in source_files:
        if not (core_path / file_path).exists():
            print(f"   ❌ Missing: {file_path}")
            results["source_files"] = False
        else:
            print(f"   ✅ Found: {file_path}")

    # 2. Check if library was built
    print("\n2. Checking built library...")
    lib_paths = [
        "workdir/LinkTarget/Library/libucpgdrivelo.dylib",
        "instdir/LibreOfficeDev.app/Contents/Frameworks/libucpgdrivelo.dylib"
    ]

    for lib_path in lib_paths:
        full_path = core_path / lib_path
        if full_path.exists():
            lib_size = full_path.stat().st_size
            print(f"   ✅ Found library: {lib_path} ({lib_size:,} bytes)")
            results["library_built"] = True
            break
        else:
            print(f"   ❌ Not found: {lib_path}")

    # 3. Check component file
    print("\n3. Checking component file...")
    component_path = core_path / "ucb/source/ucp/gdrive/ucpgdrive.component"
    if component_path.exists():
        print(f"   ✅ Found: {component_path}")
        results["component_file"] = True
    else:
        print(f"   ❌ Missing: {component_path}")

    # 4. Check installation
    print("\n4. Checking installation...")
    install_paths = [
        "instdir/LibreOfficeDev.app/Contents/Frameworks/libucpgdrivelo.dylib",
        "instdir/LibreOfficeDev.app/Contents/Resources/program/ucpgdrive.component"
    ]

    installed_count = 0
    for install_path in install_paths:
        full_path = core_path / install_path
        if full_path.exists():
            print(f"   ✅ Installed: {install_path}")
            installed_count += 1
        else:
            print(f"   ❌ Not installed: {install_path}")

    results["installation"] = installed_count > 0

    # 5. Summary
    print("\n" + "=" * 60)
    print("📊 BUILD STATUS SUMMARY")
    print("=" * 60)

    total_checks = len(results)
    passed_checks = sum(results.values())

    for check, status in results.items():
        status_icon = "✅" if status else "❌"
        print(f"{status_icon} {check.replace('_', ' ').title()}: {'PASS' if status else 'FAIL'}")

    success_rate = (passed_checks / total_checks) * 100
    print(f"\nOverall Status: {passed_checks}/{total_checks} checks passed ({success_rate:.1f}%)")

    if results["library_built"]:
        print("\n🎉 SUCCESS: Google Drive library built successfully!")
        return True
    else:
        print("\n⚠️  INCOMPLETE: Google Drive library not yet built")
        return False

if __name__ == "__main__":
    success = check_build_status()
    sys.exit(0 if success else 1)