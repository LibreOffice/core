# Slack Integration - "Share to Slack" Feature

**Date:** July 27, 2025  
**Feature:** LibreOffice "Share to Slack" functionality  
**Status:** Planning and Research Phase  

## 📋 **Project Overview**

Implement a "Share to Slack" feature for LibreOffice that allows users to upload documents directly to Slack channels, direct messages, or threads. This builds upon the successful Google Drive and Dropbox cloud storage integrations already implemented in this codebase.

### **Core User Experience**
1. User has a LibreOffice document open (Writer, Calc, Impress, etc.)
2. User selects "Share to Slack" from File menu or context menu
3. OAuth2 authentication flow (if not already authenticated)
4. User selects target workspace, channel/DM, and optional message
5. Document uploads to Slack in native LibreOffice format
6. Success confirmation with link to shared file in Slack

## 🎯 **Key Design Decisions**

### **Format Preservation Priority** ✅
**Decision**: Prioritize native LibreOffice format preservation over Slack preview compatibility

**Rationale**:
- Maintains document integrity (styles, macros, formatting)
- No data loss from conversion processes
- Consistent with existing cloud storage behavior
- Simpler implementation without conversion pipelines
- Users understand they're sharing LibreOffice documents

**Trade-off**: Recipients need LibreOffice or compatible software to view documents

## 🔍 **Technical Research Findings**

### **Slack OAuth2 Authentication** ✅ **COMPATIBLE**

**Authentication Flow**: Standard OAuth2 (compatible with existing patterns)
- **Authorization URL**: `https://slack.com/oauth/v2/authorize`
- **Token Exchange**: `https://slack.com/api/oauth.v2.access`
- **Pattern**: Nearly identical to Google Drive/Dropbox implementations

**Required Scopes**:
- `files:write` - Upload files to Slack
- `chat:write` - Post messages with files  
- `channels:read` - List available channels for selection

**Integration**: Can reuse existing OAuth2HttpServer architecture with Slack-specific endpoints

### **File Upload API** ⚠️ **NEW 2-STEP PROCESS**

**Critical Update (2024)**: Slack deprecated `files.upload` method
- **Sunset Date**: November 12, 2025
- **New Required Flow**: Asynchronous 2-step upload process

**New Upload Workflow**:
1. **`files.getUploadURLExternal`** - Get temporary upload URL
2. **Direct file upload** - Upload to provided URL
3. **`files.completeUploadExternal`** - Finalize upload in Slack

**Benefits**: More reliable for large files, better infrastructure scaling

### **File Size and Type Restrictions**

**File Size Limits**:
- **Snippets**: 1MB limit (code/text files)
- **Regular Files**: ~5GB workspace limit (Free plan)
- **No explicit per-file limit** for regular uploads

**File Type Support**:
- **Native LibreOffice formats**: .odt, .ods, .odp, .odg supported
- **Auto-detection**: Slack infers file type from filename and magic bytes
- **Admin restrictions**: Some workspaces may block certain file types
- **Slack Connect**: Additional restrictions for cross-workspace sharing

**LibreOffice Impact**: Most documents well under limits, large presentations may approach restrictions

### **Sharing Target Options** 🎯

**Supported Targets**:
- **Public Channels**: `#general`, `#random`, etc.
- **Private Channels**: If bot has access
- **Direct Messages**: 1:1 conversations  
- **Group DMs**: Multi-person private chats
- **Thread Replies**: Reply to existing messages using `thread_ts`

**Target Selection**: Users specify via channel names, IDs, or user IDs

## 🏗️ **Implementation Architecture**

### **Code Organization** (Building on Existing Patterns)

```
ucb/source/ucp/slack/
├── SlackApiClient.cxx/hxx          ← Similar to GoogleDriveApiClient
├── SlackOAuth2Server.cxx/hxx       ← Reuse OAuth2HttpServer pattern
├── slack_json.cxx/hxx              ← JSON parsing for Slack API responses
├── slack_provider.cxx/hxx          ← UCB provider (if needed)
└── slack_content.cxx/hxx           ← UCB content handling (if needed)

sfx2/source/dialog/
├── SlackShareDialog.cxx/hxx        ← Similar to DropboxDialog
└── slackshare.ui                   ← UI dialog definition

sfx2/source/appl/
└── appopen.cxx                     ← Add "Share to Slack" menu integration
```

### **Configuration Integration**

```cpp
// config_oauth2.h.in additions:
#define SLACK_CLIENT_ID "your_slack_app_client_id"
#define SLACK_CLIENT_SECRET "your_slack_client_secret"
#define SLACK_AUTH_URL "https://slack.com/oauth/v2/authorize"
#define SLACK_TOKEN_URL "https://slack.com/api/oauth.v2.access"
#define SLACK_API_BASE "https://slack.com/api"
#define SLACK_SCOPES "files:write,chat:write,channels:read"
```

### **Build System Integration**

```makefile
# ucb/Library_ucpslack.mk (new library)
# Similar to Library_ucpoauth2.mk structure
```

## 🎨 **User Interface Design**

### **Menu Integration**
- **Location**: File menu → "Share to Slack..." (alongside cloud storage options)
- **Alternative**: Right-click context menu option
- **Availability**: When document is open and saved

### **Share Dialog Layout**
```
┌─ Share to Slack ─────────────────────────────────┐
│ Document: Budget_2025.ods (847 KB)               │
│                                                  │
│ Workspace: [Acme Corp                    ▼]      │
│ Share to:  [#finance                     ▼]      │
│                                                  │
│ Message (optional):                              │
│ ┌──────────────────────────────────────────────┐ │
│ │ Q4 budget ready for review                   │ │
│ └──────────────────────────────────────────────┘ │
│                                                  │
│ ☐ Share as thread reply to existing message     │
│                                                  │
│                    [Cancel]    [Share]           │
└──────────────────────────────────────────────────┘
```

### **Authentication Flow**
1. **First Use**: OAuth2 browser redirect → Slack workspace selection → Permission grant
2. **Subsequent Uses**: Automatic with stored tokens
3. **Token Refresh**: Seamless background refresh when tokens expire

## 📊 **Implementation Phases**

### **Phase 1: MVP Core Functionality** 🎯 **PRIMARY FOCUS**

**Essential Features**:
- ✅ OAuth2 authentication with Slack
- ✅ Workspace and channel selection
- ✅ Single document upload with new async API
- ✅ Basic error handling and user feedback
- ✅ File menu integration

**Success Criteria**:
- User can authenticate with Slack workspace
- User can select target channel/DM
- LibreOffice document uploads successfully  
- File appears in Slack with proper name and format
- Error messages provide actionable feedback

### **Phase 2: Enhanced User Experience**

**Additional Features**:
- Multiple workspace support
- Recent channels memory/favorites
- Progress indicators for large file uploads
- Thread reply functionality
- Drag-and-drop from LibreOffice to Slack (stretch goal)

### **Phase 3: Advanced Integration**

**Future Enhancements**:
- Batch document sharing
- Document update notifications to Slack
- Collaboration workflow hooks
- Template sharing workflows
- Integration with LibreOffice collaboration features

## 🔧 **Technical Considerations**

### **Security and Privacy**
- **Token Storage**: Secure credential storage using existing patterns
- **Sensitive Documents**: Consider warning dialogs for confidential content
- **Audit Trail**: Log sharing activities for compliance
- **Permissions**: Respect LibreOffice document restrictions/DRM

### **Error Handling Scenarios**
- **Network Failures**: Retry logic with exponential backoff
- **Authentication Errors**: Clear re-authentication flows
- **Permission Errors**: Helpful messages when user lacks channel access
- **File Size Errors**: Clear guidance on size limits
- **Workspace Restrictions**: Handle admin-blocked file types gracefully

### **Performance Considerations**
- **Large Files**: Progress indicators and cancellation support
- **Background Uploads**: Consider async upload with notifications
- **Memory Usage**: Efficient file streaming for large documents
- **Network Optimization**: Chunked uploads for reliability

## 🧪 **Testing Strategy**

### **Unit Testing**
- OAuth2 flow components
- API client methods
- JSON parsing for Slack responses
- Error handling scenarios

### **Integration Testing**
- End-to-end upload workflow
- Multi-workspace scenarios
- Various file types and sizes
- Network failure recovery

### **User Testing**
- Workflow usability
- Error message clarity
- Performance with typical document sizes
- Cross-platform compatibility

## 📝 **Open Questions and Decisions Needed**

### **Implementation Questions**
1. **UCB Integration**: Do we need full UCB provider or just API client for sharing?
2. **Dialog Framework**: Reuse existing dialog patterns or create new Slack-specific UI?
3. **Token Persistence**: Store per-workspace tokens or global Slack tokens?
4. **Multi-workspace UX**: How to handle users in multiple Slack workspaces?

### **User Experience Questions**
1. **Default Behavior**: Remember last-used channel or always prompt?
2. **Confirmation**: Show success notification or open Slack in browser?
3. **File Naming**: Use document title or filename for Slack upload?
4. **Batch Operations**: Support selecting multiple documents for sharing?

### **Technical Questions**
1. **Library Dependencies**: Any additional dependencies needed for Slack API?
2. **Build Integration**: New library or extend existing OAuth2 library?
3. **Platform Support**: Any platform-specific considerations for OAuth2 flow?

## 📚 **Reference Implementation**

### **Existing Code to Study**
- **Google Drive**: `ucb/source/ucp/gdrive/GoogleDriveApiClient.cxx` - OAuth2 and API patterns
- **Dropbox**: `ucb/source/ucp/dropbox/DropboxApiClient.cxx` - File upload workflows  
- **OAuth2 Server**: `ucb/source/ucp/dropbox/oauth2_http_server.cxx` - HTTP callback handling
- **Dialog UI**: `sfx2/source/dialog/dropboxdialog.cxx` - User interface patterns

### **Slack API Documentation**
- **OAuth2 Guide**: https://api.slack.com/authentication/oauth-v2
- **File Upload (New)**: https://docs.slack.dev/changelog/2024-04-a-better-way-to-upload-files-is-here-to-stay
- **API Methods**: https://api.slack.com/methods
- **Scopes**: https://api.slack.com/scopes

## 🚀 **Next Steps**

### **Immediate Actions**
1. **📁 Create base directory structure**: `ucb/source/ucp/slack/`
2. **🔍 Study existing patterns**: Review GoogleDriveApiClient and DropboxApiClient implementations
3. **⚙️ Setup build configuration**: Create Library_ucpslack.mk
4. **🔑 Implement OAuth2 foundation**: Adapt existing OAuth2HttpServer for Slack endpoints

### **Development Sequence**
1. **SlackApiClient skeleton** with OAuth2 authentication
2. **Basic workspace/channel listing** API calls
3. **File upload workflow** using new async API
4. **Dialog UI implementation** for channel selection
5. **File menu integration** and command handling
6. **Error handling and user feedback** polish
7. **Testing and validation** with real Slack workspaces

---

## 📈 **Success Metrics**

**Technical Success**:
- ✅ Clean integration with existing LibreOffice architecture
- ✅ Successful file uploads to Slack using new async API
- ✅ Robust error handling and recovery
- ✅ Secure authentication and token management

**User Experience Success**:
- ✅ Intuitive workflow matching cloud storage patterns
- ✅ Fast and reliable document sharing
- ✅ Clear feedback for all user actions
- ✅ Minimal authentication friction

**Business Impact**:
- ✅ Enhanced LibreOffice collaboration capabilities
- ✅ Competitive feature parity with modern office suites
- ✅ Improved team workflow integration
- ✅ Foundation for future Slack collaboration features

---

*This document will be updated throughout the implementation process to reflect progress, decisions, and any architectural changes.*
