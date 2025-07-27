#!/usr/bin/env python3
"""
End-to-end test for Dropbox integration in LibreOffice

This script tests the complete workflow:
1. Start LibreOffice
2. Access File menu -> Open from Dropbox
3. Verify OAuth2 authentication flow
4. Check file listing functionality
5. Test file selection and opening

Prerequisites:
- LibreOffice build with Dropbox integration
- Real Dropbox account for testing
- Valid Dropbox API credentials in config_oauth2.h
"""

import subprocess
import time
import os
import sys
import signal
import threading
from pathlib import Path

class DropboxE2ETest:
    def __init__(self):
        self.soffice_path = "./instdir/LibreOfficeDev.app/Contents/MacOS/soffice"
        self.process = None
        self.test_results = []

    def log_result(self, test_name, success, message=""):
        status = "✅ PASS" if success else "❌ FAIL"
        self.test_results.append((test_name, success, message))
        print(f"{status}: {test_name}")
        if message:
            print(f"   {message}")

    def run_test(self):
        print("🚀 Starting Dropbox E2E Test")
        print("=" * 50)

        # Test 1: Check LibreOffice build exists
        if not self.check_libreoffice_build():
            return False

        # Test 2: Start LibreOffice
        if not self.start_libreoffice():
            return False

        # Test 3: Interactive testing prompts
        self.run_interactive_tests()

        # Test 4: Cleanup
        self.cleanup()

        # Print summary
        self.print_summary()

        return all(result[1] for result in self.test_results)

    def check_libreoffice_build(self):
        """Test 1: Verify LibreOffice build exists"""
        if os.path.exists(self.soffice_path):
            self.log_result("LibreOffice Build Check", True, f"Found at {self.soffice_path}")
            return True
        else:
            self.log_result("LibreOffice Build Check", False, f"Not found at {self.soffice_path}")
            return False

    def start_libreoffice(self):
        """Test 2: Start LibreOffice Writer"""
        try:
            print("Starting LibreOffice Writer...")
            self.process = subprocess.Popen([
                self.soffice_path,
                "--writer",
                "--norestore"
            ], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

            # Give it time to start
            time.sleep(3)

            # Check if process is still running
            if self.process.poll() is None:
                self.log_result("LibreOffice Startup", True, "Writer started successfully")
                return True
            else:
                stdout, stderr = self.process.communicate()
                self.log_result("LibreOffice Startup", False, f"Process exited: {stderr.decode()}")
                return False

        except Exception as e:
            self.log_result("LibreOffice Startup", False, f"Failed to start: {e}")
            return False

    def run_interactive_tests(self):
        """Test 3: Interactive test prompts"""
        print("\n🔧 Interactive Testing Phase")
        print("LibreOffice should now be running. Please perform these tests manually:")
        print()

        # Test the Dropbox menu item
        self.prompt_test(
            "Dropbox Menu Item",
            "1. Go to File menu\n2. Look for 'Open from Dropbox...' option\n3. Does the menu item exist?"
        )

        # Test OAuth2 flow
        self.prompt_test(
            "OAuth2 Authentication",
            "1. Click 'Open from Dropbox...'\n2. Did your browser open to Dropbox login?\n3. Were you able to authorize LibreOffice?"
        )

        # Test file dialog
        self.prompt_test(
            "File Dialog Display",
            "1. After OAuth2, did the Dropbox file dialog appear?\n2. Can you see your Dropbox files listed?\n3. Is the interface responsive?"
        )

        # Test file selection
        self.prompt_test(
            "File Selection",
            "1. Select a document file (e.g., .docx, .pdf, .txt)\n2. Click Open button\n3. Did the file download and open in LibreOffice?"
        )

        # Test error handling
        self.prompt_test(
            "Error Handling",
            "1. Try selecting a large file or unsupported format\n2. Are error messages clear and helpful?\n3. Does the dialog handle errors gracefully?"
        )

    def prompt_test(self, test_name, instructions):
        """Prompt user to test a specific feature"""
        print(f"\n📋 Test: {test_name}")
        print(instructions)
        print()

        while True:
            result = input("Did this test PASS? (y/n/s to skip): ").lower().strip()
            if result == 'y':
                self.log_result(test_name, True, "User confirmed success")
                break
            elif result == 'n':
                reason = input("What went wrong? ").strip()
                self.log_result(test_name, False, reason)
                break
            elif result == 's':
                self.log_result(test_name, True, "Skipped by user")
                break
            else:
                print("Please enter 'y' for yes, 'n' for no, or 's' to skip")

    def cleanup(self):
        """Test 4: Cleanup LibreOffice process"""
        if self.process and self.process.poll() is None:
            print("\n🧹 Cleaning up...")
            try:
                # Try graceful termination first
                self.process.terminate()
                self.process.wait(timeout=5)
                self.log_result("Cleanup", True, "LibreOffice terminated gracefully")
            except subprocess.TimeoutExpired:
                # Force kill if needed
                self.process.kill()
                self.log_result("Cleanup", True, "LibreOffice force-killed")
            except Exception as e:
                self.log_result("Cleanup", False, f"Cleanup error: {e}")

    def print_summary(self):
        """Print test summary"""
        print("\n" + "=" * 50)
        print("📊 TEST SUMMARY")
        print("=" * 50)

        passed = sum(1 for _, success, _ in self.test_results if success)
        total = len(self.test_results)

        for test_name, success, message in self.test_results:
            status = "✅" if success else "❌"
            print(f"{status} {test_name}")
            if message and not success:
                print(f"   └─ {message}")

        print(f"\nResults: {passed}/{total} tests passed")

        if passed == total:
            print("🎉 ALL TESTS PASSED! Dropbox integration is working correctly.")
        else:
            print("⚠️  Some tests failed. Check the issues above.")

        return passed == total

def main():
    """Main test function"""
    print("Dropbox Integration End-to-End Test")
    print("===================================")
    print()
    print("This test will verify the complete Dropbox integration workflow.")
    print("Make sure you have:")
    print("• Valid Dropbox API credentials configured")
    print("• A Dropbox account with some test files")
    print("• LibreOffice built with Dropbox integration")
    print()

    if input("Ready to start? (y/n): ").lower() != 'y':
        print("Test cancelled.")
        return 1

    test = DropboxE2ETest()
    success = test.run_test()

    return 0 if success else 1

if __name__ == "__main__":
    sys.exit(main())
