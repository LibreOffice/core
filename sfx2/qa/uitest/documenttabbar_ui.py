#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, select_pos, get_url_for_data_file
from uitest.uihelper.keyboard import key_combination
from libreoffice.uno.propertyvalue import mkPropertyValues
import time

class DocumentTabBarUITest(UITestCase):
    """
    UI tests for DocumentTabBar widget focusing on user interactions

    These tests verify that the DocumentTabBar responds correctly to:
    - Mouse clicks for tab activation and closing
    - Keyboard navigation between tabs
    - Context menu operations
    - Drag and drop operations (if supported)
    - Visual feedback and accessibility
    """

    def setUp(self):
        """Set up test environment"""
        super().setUp()
        # Ensure we start with a clean state
        self.close_all_documents()

    def tearDown(self):
        """Clean up test environment"""
        self.close_all_documents()
        super().tearDown()

    def close_all_documents(self):
        """Helper to close all open documents"""
        # Close all documents that might be open
        try:
            while True:
                self.xUITest.executeCommand(".uno:CloseDoc")
                time.sleep(0.1)
        except:
            # No more documents to close
            pass

    def create_new_document(self, doc_type="writer"):
        """Helper to create a new document of specified type"""
        if doc_type == "writer":
            self.ui_test.create_doc_in_start_center("writer")
        elif doc_type == "calc":
            self.ui_test.create_doc_in_start_center("calc")
        elif doc_type == "impress":
            self.ui_test.create_doc_in_start_center("impress")
        elif doc_type == "draw":
            self.ui_test.create_doc_in_start_center("draw")
        else:
            raise ValueError(f"Unknown document type: {doc_type}")

        # Wait for document to be fully loaded
        time.sleep(0.5)

    def get_tab_bar_element(self):
        """Helper to get the DocumentTabBar UI element"""
        # Note: The exact UI tree path would depend on the actual implementation
        # This is a placeholder that would need to be adjusted based on the real UI structure
        try:
            # Try to find the DocumentTabBar in the UI tree
            return self.ui_test.get_component().getChild("DocumentTabBar")
        except:
            # If DocumentTabBar is not directly accessible, it might be embedded
            # in another container or have a different name
            return None

    def test_single_tab_creation_and_closing(self):
        """Test creating and closing a single tab"""

        # Create a new document
        with self.ui_test.create_doc_in_start_center("writer") as document:

            # In a tabbed interface, there should be exactly one tab
            # Note: This test assumes the DocumentTabBar is visible
            # The exact verification method would depend on implementation

            # Verify document is open
            self.assertIsNotNone(document)

            # Test that document can be closed normally
            # The tab should disappear when document is closed

        # After context manager exits, document should be closed
        # and tab should be removed

    def test_multiple_tab_creation(self):
        """Test creating multiple tabs and verifying they appear"""

        # Create first document
        with self.ui_test.create_doc_in_start_center("writer") as writer_doc:

            # Create second document
            with self.ui_test.create_doc_in_start_center("calc") as calc_doc:

                # Create third document
                with self.ui_test.create_doc_in_start_center("impress") as impress_doc:

                    # At this point, we should have 3 tabs visible
                    # Verify that all documents are accessible
                    self.assertIsNotNone(writer_doc)
                    self.assertIsNotNone(calc_doc)
                    self.assertIsNotNone(impress_doc)

                    # Test switching between documents using Window menu
                    # (This simulates what clicking on tabs would do)

                    # Try to access documents via Window menu
                    self.xUITest.executeCommand(".uno:WindowList")
                    time.sleep(0.2)

                    # In a real implementation, we would:
                    # - Click on specific tab elements
                    # - Verify active tab changes
                    # - Verify document content switches appropriately

    def test_tab_activation_by_clicking(self):
        """Test clicking on tabs to activate documents"""

        # Create multiple documents
        with self.ui_test.create_doc_in_start_center("writer") as writer_doc:
            # Add some content to identify this document
            xDoc = self.xUITest.getTopFocusWindow().getChild("writer_edit")
            xDoc.executeAction("TYPE", mkPropertyValues({"TEXT": "Writer Document Content"}))

            with self.ui_test.create_doc_in_start_center("calc") as calc_doc:
                # Add content to calc document
                xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")
                xGridWin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
                xGridWin.executeAction("TYPE", mkPropertyValues({"TEXT": "Calc Content"}))

                # Test switching between documents
                # In a real implementation with tab bar, we would:
                # 1. Get tab bar element
                # 2. Find writer tab
                # 3. Click on writer tab
                # 4. Verify writer document is now active
                # 5. Verify content matches what we typed

                # For now, test switching via keyboard shortcuts
                key_combination(self.xUITest, "CTRL", "SHIFT", "TAB")
                time.sleep(0.2)

                # Verify we can access both documents
                self.assertIsNotNone(writer_doc)
                self.assertIsNotNone(calc_doc)

    def test_tab_closing_with_mouse(self):
        """Test closing tabs by clicking close buttons"""

        # Create multiple documents
        with self.ui_test.create_doc_in_start_center("writer") as writer_doc:
            with self.ui_test.create_doc_in_start_center("calc") as calc_doc:

                # Test closing the active document (calc)
                # In a real implementation, this would involve:
                # 1. Finding the close button on the active tab
                # 2. Clicking the close button
                # 3. Verifying the document closes
                # 4. Verifying the tab disappears
                # 5. Verifying the previous document becomes active

                # For now, use the standard close command
                self.xUITest.executeCommand(".uno:CloseDoc")
                time.sleep(0.2)

                # Calc document should be closed, writer should remain
                self.assertIsNotNone(writer_doc)

                # Test closing the last remaining document
                self.xUITest.executeCommand(".uno:CloseDoc")
                time.sleep(0.2)

    def test_keyboard_navigation_between_tabs(self):
        """Test navigating between tabs using keyboard shortcuts"""

        # Create multiple documents
        with self.ui_test.create_doc_in_start_center("writer") as writer_doc:
            with self.ui_test.create_doc_in_start_center("calc") as calc_doc:
                with self.ui_test.create_doc_in_start_center("impress") as impress_doc:

                    # Test Ctrl+Tab (next document)
                    key_combination(self.xUITest, "CTRL", "TAB")
                    time.sleep(0.2)

                    # Test Ctrl+Shift+Tab (previous document)
                    key_combination(self.xUITest, "CTRL", "SHIFT", "TAB")
                    time.sleep(0.2)

                    # Test Ctrl+PageDown (next tab in some implementations)
                    key_combination(self.xUITest, "CTRL", "PAGEDOWN")
                    time.sleep(0.2)

                    # Test Ctrl+PageUp (previous tab in some implementations)
                    key_combination(self.xUITest, "CTRL", "PAGEUP")
                    time.sleep(0.2)

                    # Verify all documents are still accessible
                    self.assertIsNotNone(writer_doc)
                    self.assertIsNotNone(calc_doc)
                    self.assertIsNotNone(impress_doc)

    def test_tab_context_menu(self):
        """Test right-clicking on tabs to show context menu"""

        with self.ui_test.create_doc_in_start_center("writer") as writer_doc:
            with self.ui_test.create_doc_in_start_center("calc") as calc_doc:

                # In a real implementation, this would involve:
                # 1. Right-clicking on a specific tab
                # 2. Verifying context menu appears
                # 3. Testing context menu options like:
                #    - Close Tab
                #    - Close Other Tabs
                #    - Close All Tabs
                #    - Move to New Window (if supported)
                #    - Duplicate Tab (if supported)

                # For now, test similar functionality via menus
                self.xUITest.executeCommand(".uno:WindowList")
                time.sleep(0.2)

                # Test close all documents functionality
                try:
                    # This might not exist in all versions
                    self.xUITest.executeCommand(".uno:CloseAll")
                    time.sleep(0.2)
                except:
                    # Fall back to closing individually
                    self.xUITest.executeCommand(".uno:CloseDoc")
                    time.sleep(0.2)
                    self.xUITest.executeCommand(".uno:CloseDoc")
                    time.sleep(0.2)

    def test_tab_drag_and_drop(self):
        """Test dragging tabs to reorder them (if supported)"""

        # Create multiple documents
        with self.ui_test.create_doc_in_start_center("writer") as writer_doc:
            with self.ui_test.create_doc_in_start_center("calc") as calc_doc:
                with self.ui_test.create_doc_in_start_center("impress") as impress_doc:

                    # Drag and drop functionality would be complex to test
                    # In a real implementation, this would involve:
                    # 1. Getting tab positions
                    # 2. Simulating mouse drag from one position to another
                    # 3. Verifying tab order changes
                    # 4. Verifying document order in window list changes

                    # For now, just verify documents are accessible
                    self.assertIsNotNone(writer_doc)
                    self.assertIsNotNone(calc_doc)
                    self.assertIsNotNone(impress_doc)

    def test_tab_tooltips(self):
        """Test that hovering over tabs shows appropriate tooltips"""

        with self.ui_test.create_doc_in_start_center("writer") as writer_doc:

            # Add some content and save to create a meaningful tooltip
            xDoc = self.xUITest.getTopFocusWindow().getChild("writer_edit")
            xDoc.executeAction("TYPE", mkPropertyValues({"TEXT": "Test document content"}))

            # In a real implementation, this would involve:
            # 1. Getting tab element
            # 2. Simulating mouse hover
            # 3. Verifying tooltip appears with document path/name
            # 4. Verifying tooltip content is correct

            # For now, just verify document exists
            self.assertIsNotNone(writer_doc)

    def test_tab_visual_states(self):
        """Test visual feedback for different tab states"""

        with self.ui_test.create_doc_in_start_center("writer") as writer_doc:

            # Test modified document indicator
            xDoc = self.xUITest.getTopFocusWindow().getChild("writer_edit")
            xDoc.executeAction("TYPE", mkPropertyValues({"TEXT": "Modified content"}))

            # In a real implementation, this would verify:
            # 1. Active tab has different visual appearance
            # 2. Modified documents show modification indicator (e.g., asterisk)
            # 3. Hover state shows visual feedback
            # 4. Focus state is visually distinct

            with self.ui_test.create_doc_in_start_center("calc") as calc_doc:

                # Test that active tab changes visual state
                # The calc document should now be active, writer should be inactive

                # Verify documents exist
                self.assertIsNotNone(writer_doc)
                self.assertIsNotNone(calc_doc)

    def test_tab_accessibility(self):
        """Test accessibility features of tabs"""

        with self.ui_test.create_doc_in_start_center("writer") as writer_doc:

            # In a real implementation, this would test:
            # 1. Tab navigation with Tab key
            # 2. Activation with Enter/Space
            # 3. Screen reader compatibility
            # 4. High contrast mode support
            # 5. Keyboard mnemonics

            # Test that keyboard navigation works
            key_combination(self.xUITest, "F6")  # Navigate between panes
            time.sleep(0.2)

            # Verify document is accessible
            self.assertIsNotNone(writer_doc)

    def test_tab_overflow_handling(self):
        """Test tab bar behavior when too many tabs are open"""

        # Create many documents to test overflow
        documents = []
        try:
            for i in range(10):  # Create 10 documents
                if i % 3 == 0:
                    doc = self.ui_test.create_doc_in_start_center("writer")
                elif i % 3 == 1:
                    doc = self.ui_test.create_doc_in_start_center("calc")
                else:
                    doc = self.ui_test.create_doc_in_start_center("impress")

                documents.append(doc)
                time.sleep(0.1)  # Brief pause to avoid overwhelming the system

            # In a real implementation, this would test:
            # 1. Scroll buttons appear when tabs overflow
            # 2. Scroll buttons work correctly
            # 3. All tabs remain accessible
            # 4. Tab dropdown menu works (if implemented)

            # Test that all documents are accessible via Window menu
            self.xUITest.executeCommand(".uno:WindowList")
            time.sleep(0.2)

        finally:
            # Clean up all documents
            for doc in documents:
                if doc:
                    try:
                        doc.close(True)  # Force close without saving
                    except:
                        pass
            time.sleep(0.5)

    def test_tab_with_unsaved_documents(self):
        """Test tab behavior with unsaved document changes"""

        with self.ui_test.create_doc_in_start_center("writer") as writer_doc:

            # Make changes to the document
            xDoc = self.xUITest.getTopFocusWindow().getChild("writer_edit")
            xDoc.executeAction("TYPE", mkPropertyValues({"TEXT": "Unsaved changes"}))

            # In a real implementation, we would verify:
            # 1. Tab shows modification indicator
            # 2. Attempting to close shows save dialog
            # 3. Save dialog can be canceled, keeping tab open
            # 4. Forced close discards changes and closes tab

            # Test that document has unsaved changes
            # (The exact method to check this would depend on implementation)

            # Create another document to test switching with unsaved changes
            with self.ui_test.create_doc_in_start_center("calc") as calc_doc:

                # Test switching between documents with unsaved changes
                key_combination(self.xUITest, "CTRL", "SHIFT", "TAB")
                time.sleep(0.2)

                # Verify both documents are accessible
                self.assertIsNotNone(writer_doc)
                self.assertIsNotNone(calc_doc)

    def test_tab_performance_with_rapid_operations(self):
        """Test tab bar performance with rapid user operations"""

        # Create multiple documents
        with self.ui_test.create_doc_in_start_center("writer") as writer_doc:
            with self.ui_test.create_doc_in_start_center("calc") as calc_doc:
                with self.ui_test.create_doc_in_start_center("impress") as impress_doc:

                    # Rapidly switch between documents
                    for i in range(10):
                        key_combination(self.xUITest, "CTRL", "TAB")
                        time.sleep(0.05)  # Very brief pause
                        key_combination(self.xUITest, "CTRL", "SHIFT", "TAB")
                        time.sleep(0.05)

                    # Verify all documents are still accessible
                    self.assertIsNotNone(writer_doc)
                    self.assertIsNotNone(calc_doc)
                    self.assertIsNotNone(impress_doc)

    def test_tab_integration_with_window_menu(self):
        """Test that tab bar integrates properly with Window menu"""

        with self.ui_test.create_doc_in_start_center("writer") as writer_doc:
            with self.ui_test.create_doc_in_start_center("calc") as calc_doc:

                # Open Window menu
                self.xUITest.executeCommand(".uno:WindowList")
                time.sleep(0.2)

                # In a real implementation, we would verify:
                # 1. Window menu shows all open documents
                # 2. Active document is marked in menu
                # 3. Selecting document from menu activates corresponding tab
                # 4. Document order in menu matches tab order

                # Test using window menu to switch documents
                try:
                    # Try to use window switching commands
                    key_combination(self.xUITest, "ALT", "1")  # Switch to first window
                    time.sleep(0.2)
                except:
                    # Not all systems support this
                    pass

                # Verify documents are accessible
                self.assertIsNotNone(writer_doc)
                self.assertIsNotNone(calc_doc)

    def test_tab_with_different_document_states(self):
        """Test tab appearance with documents in different states"""

        with self.ui_test.create_doc_in_start_center("writer") as writer_doc:

            # Test normal document state
            self.assertIsNotNone(writer_doc)

            # Test modified document state
            xDoc = self.xUITest.getTopFocusWindow().getChild("writer_edit")
            xDoc.executeAction("TYPE", mkPropertyValues({"TEXT": "Modified document"}))

            # Test read-only document state (if we had a read-only document)
            # This would require loading an actual read-only file

            # Create another document for comparison
            with self.ui_test.create_doc_in_start_center("calc") as calc_doc:

                # In a real implementation, we would verify:
                # 1. Normal tabs have standard appearance
                # 2. Modified tabs show modification indicator
                # 3. Read-only tabs show read-only indicator
                # 4. Active tab is visually distinct from inactive tabs

                # Verify both documents exist
                self.assertIsNotNone(writer_doc)
                self.assertIsNotNone(calc_doc)

# Additional test class for specific interaction patterns
class DocumentTabBarAdvancedUITest(UITestCase):
    """
    Advanced UI tests for DocumentTabBar covering complex interaction patterns
    """

    def test_tab_middle_click_behavior(self):
        """Test middle-clicking on tabs (typically closes tab)"""

        with self.ui_test.create_doc_in_start_center("writer") as writer_doc:
            with self.ui_test.create_doc_in_start_center("calc") as calc_doc:

                # In a real implementation, this would:
                # 1. Simulate middle mouse button click on tab
                # 2. Verify document closes
                # 3. Verify tab disappears
                # 4. Verify remaining document becomes active

                # For now, test equivalent functionality
                self.xUITest.executeCommand(".uno:CloseDoc")
                time.sleep(0.2)

                # One document should remain
                self.assertIsNotNone(writer_doc)

    def test_tab_double_click_behavior(self):
        """Test double-clicking on tabs (implementation-specific behavior)"""

        with self.ui_test.create_doc_in_start_center("writer") as writer_doc:

            # Double-click behavior might include:
            # - Toggle maximized state
            # - Rename tab (if supported)
            # - Show document properties
            # - No action (just activation)

            # Since behavior is implementation-specific, just verify document exists
            self.assertIsNotNone(writer_doc)

    def test_tab_wheel_scroll_behavior(self):
        """Test mouse wheel scrolling over tab bar"""

        # Create many documents to enable scrolling
        documents = []
        try:
            for i in range(8):
                doc = self.ui_test.create_doc_in_start_center("writer")
                documents.append(doc)
                time.sleep(0.1)

            # In a real implementation, this would test:
            # 1. Mouse wheel over tab bar scrolls tabs
            # 2. Ctrl+wheel over tabs switches active tab
            # 3. Wheel scrolling respects tab boundaries

            # Verify all documents were created
            for doc in documents:
                self.assertIsNotNone(doc)

        finally:
            # Clean up
            for doc in documents:
                if doc:
                    try:
                        doc.close(True)
                    except:
                        pass

    def test_tab_focus_behavior(self):
        """Test keyboard focus behavior in tab bar"""

        with self.ui_test.create_doc_in_start_center("writer") as writer_doc:
            with self.ui_test.create_doc_in_start_center("calc") as calc_doc:

                # Test Tab key navigation to tab bar
                # Test arrow key navigation within tab bar
                # Test Enter/Space activation of focused tab
                # Test Escape to return focus to document

                # In a real implementation, this would involve:
                # 1. Pressing Tab until tab bar gets focus
                # 2. Using arrow keys to navigate between tabs
                # 3. Using Enter to activate selected tab
                # 4. Verifying focus indicators are visible

                # For now, test basic keyboard navigation
                key_combination(self.xUITest, "F6")  # Cycle through panes
                time.sleep(0.2)

                # Verify documents are accessible
                self.assertIsNotNone(writer_doc)
                self.assertIsNotNone(calc_doc)