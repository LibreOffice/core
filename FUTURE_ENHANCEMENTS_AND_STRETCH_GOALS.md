# Future Enhancements and Stretch Goals

## Overview

This document catalogs all the cosmetic improvements, stretch goals, and features that were **not implemented** during the LibreOffice integration projects. While the core functionality has been successfully delivered, these items represent opportunities for future development and user experience improvements.

---

## 🖱️ **Document Tab Bar Features (Partially Complete)**

### ✅ **What Was Implemented**
- Core tab bar functionality with synchronized tabs across windows
- Window switching via tab clicking
- Basic tab rendering and mouse event handling
- Integration with LibreOffice's VCL framework
- Multi-document support across Writer, Calc, and other applications

### 🔄 **Missing Tab Bar Features**
1. **Close Button Functionality**
   - Tab close buttons are drawn but not fully functional
   - Should close documents when clicked with proper confirmation dialogs

2. **Active Tab Indicator Synchronization** 
   - Tab highlighting could be improved to better reflect focused window
   - Cross-window active state synchronization needs refinement

3. **Menu Integration**
   - No menu items to enable/disable document tabbing
   - No View menu option to show/hide tab bars

4. **User Configuration Options**
   - No user preference settings for tab behavior
   - No customization of tab appearance or positioning
   - No option to configure tab bar placement (top vs bottom)

5. **Enhanced Visual Features**
   - No tab reordering via drag-and-drop
   - No tab context menus (right-click options)
   - No tab overflow handling for many documents
   - No tab thumbnails or previews on hover

---

## 💾 **Cloud Storage Saving Features**

### 🔄 **Save to Google Drive**
- **Status**: Not implemented (only opening/downloading exists)
- **Description**: Allow users to save documents directly to Google Drive
- **Implementation needs**: 
  - Upload API integration in GoogleDriveApiClient
  - File update/overwrite functionality
  - Progress indicators for large uploads
  - Conflict resolution for existing files

### 🔄 **Save to Dropbox**
- **Status**: Not implemented (only opening/downloading exists)
- **Description**: Allow users to save documents directly to Dropbox
- **Implementation needs**:
  - Upload API integration in DropboxApiClient
  - File versioning support
  - Large file upload sessions for files >150MB
  - Proper MIME type handling

### 🔄 **Save As Cloud Options**
- **Status**: Not implemented
- **Description**: Add cloud storage options to "Save As" dialog
- **Would require**: Integration with existing file picker dialogs

---

## 🎯 **User Interface Enhancements**

### 🔄 **Toolbar Buttons**
- **Status**: Not implemented
- **Description**: Quick access toolbar buttons for cloud operations
- **Missing features**:
  - "Open from Google Drive" toolbar button
  - "Open from Dropbox" toolbar button  
  - "Save to Cloud" dropdown button
  - Custom toolbar configuration options

### 🔄 **Main Menu Integration**
- **Status**: Partially implemented (only in File menu)
- **Missing locations**:
  - Recent Files submenu with cloud files
  - Templates menu with cloud template access
  - Tools menu for cloud account management
  - View menu for cloud file browser panels

### 🔄 **Redundant Authentication Dialogs**
- **Status**: Not addressed
- **Issue**: Multiple authentication prompts during single session
- **Improvement needed**: 
  - Single sign-on across all cloud operations
  - Persistent authentication state
  - Cleaner error handling for expired tokens

### 🔄 **Browser Focus Management**
- **Status**: Not implemented
- **Issue**: When OAuth2 browser opens, it doesn't automatically focus
- **Improvement needed**:
  - Automatic browser window focusing
  - Better integration with system window manager
  - Clearer user instructions for authentication flow

---

## 📁 **File Management Features**

### 🔄 **Downloaded File Naming**
- **Status**: Not corrected
- **Issue**: Downloaded files get temporary names (lu*.tmp.odt)
- **Improvement needed**:
  - Preserve original cloud file names
  - Handle name conflicts intelligently
  - Maintain file extension associations

### 🔄 **Document Exit from Tabs**
- **Status**: Not implemented
- **Description**: Close documents directly from tab interface
- **Missing functionality**:
  - Tab close buttons that actually work
  - Keyboard shortcuts for tab closure
  - Proper document save prompts before closing

### 🔄 **Cloud File Browser Panels**
- **Status**: Not implemented  
- **Description**: Persistent sidebar panels for cloud browsing
- **Would provide**:
  - Always-visible cloud file access
  - Drag-and-drop between local and cloud
  - Multi-panel file management

---

## 🧪 **Testing and Quality Assurance**

### 🔄 **Calc Application Testing**
- **Status**: Not extensively tested
- **Description**: While integration works across apps, specific Calc testing needed
- **Missing validation**:
  - Spreadsheet-specific file operations
  - Formula preservation during cloud sync
  - Large spreadsheet performance testing

### 🔄 **Cross-Application Feature Extension**
- **Status**: Basic implementation only
- **Missing applications**:
  - Draw: Cloud image and drawing storage
  - Math: Formula document cloud sync
  - Base: Database file cloud storage integration
  - Impress: Presentation template cloud access

### 🔄 **Comprehensive Testing Suite**
- **Status**: Basic testing scripts only
- **Missing test coverage**:
  - Automated regression testing
  - Performance benchmarking
  - Edge case handling (large files, slow connections)
  - Multi-user collaboration scenarios

---

## 🎨 **Visual and UX Improvements**

### 🔄 **Progress Indicators**
- **Status**: Basic implementation only
- **Missing features**:
  - Detailed upload/download progress
  - Bandwidth usage indicators
  - Estimated time remaining
  - Ability to cancel long operations

### 🔄 **File Type Recognition**
- **Status**: Basic implementation
- **Improvements needed**:
  - File type icons in cloud browsers
  - Thumbnail previews for images
  - Document preview panes
  - Better MIME type handling

### 🔄 **Cloud Account Management**
- **Status**: Not implemented
- **Missing features**:
  - Multiple account support per service
  - Account switching interface
  - Storage quota display
  - Account permission management

---

## 🔧 **Technical Infrastructure**

### 🔄 **Performance Optimizations**
- **Status**: Basic implementation
- **Missing optimizations**:
  - Intelligent caching strategies
  - Background prefetching
  - Lazy loading for large directories
  - Memory optimization for large files

### 🔄 **Offline Support**
- **Status**: Not implemented
- **Description**: Work with cloud files when offline
- **Would require**:
  - Local file caching
  - Sync conflict resolution
  - Offline mode indicators
  - Background synchronization

### 🔄 **Advanced Features**
- **Status**: Not implemented
- **Missing capabilities**:
  - File search within cloud storage
  - File sharing and collaboration features
  - Version history access
  - Real-time collaborative editing

---

## 🌐 **Google Drive Specific Features**

### ✅ **What Was Completed**
- Full OAuth2 authentication flow
- File listing and browsing
- File downloading and opening
- UCB provider integration
- End-to-end document workflow

### 🔄 **Missing Google Drive Features**
- File upload and saving
- Google Workspace document editing (Docs, Sheets, Slides)
- Shared drive support
- Permission and sharing management
- Comment and suggestion features
- Revision history access

---

## 💧 **Dropbox Specific Features**

### ✅ **What Was Completed**
- Complete OAuth2 authentication
- File listing with real-time API integration
- Successful file downloading (8814 bytes confirmed)
- Automatic file opening in LibreOffice
- JSON parsing for Dropbox API v2

### 🔄 **Missing Dropbox Features**
- File upload and saving
- Large file upload sessions (>150MB)
- Dropbox Paper integration
- File sharing and collaboration
- Dropbox Business team features
- Advanced file versioning

---

## 📊 **Priority Assessment**

### **High Priority (User-Facing)**
1. Save to Google Drive/Dropbox functionality
2. Toolbar buttons for quick access
3. Proper file naming for downloads
4. Tab close button functionality
5. Redundant authentication dialog elimination

### **Medium Priority (UX Improvements)**
1. Browser focus management during OAuth
2. Progress indicators for operations
3. Main menu integration expansion
4. Calc-specific testing and validation
5. Cloud file browser panels

### **Low Priority (Polish Features)**
1. Tab reordering and customization
2. File thumbnails and previews
3. Multiple cloud account support
4. Advanced performance optimizations
5. Offline mode support

---

## 🎯 **Conclusion**

While the core cloud storage integration functionality has been successfully implemented and is production-ready, this document represents a roadmap of enhancements that could significantly improve the user experience. The features listed here range from simple UI polish to complex architectural additions.

**Current Status**: ✅ **Core functionality complete and working**
**Future Opportunities**: 🔄 **Rich enhancement pipeline available**

The foundation is solid, and these enhancements could be implemented incrementally based on user feedback and development priorities.
