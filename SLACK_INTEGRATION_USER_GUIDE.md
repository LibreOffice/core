# LibreOffice Slack Integration - User Guide

## Overview

The Slack integration allows you to share documents directly from LibreOffice to Slack channels and direct messages. This seamless integration eliminates the need to save files locally and manually upload them to Slack.

## Getting Started

### System Requirements
- **LibreOffice**: Version with Slack integration enabled
- **Internet Connection**: Required for Slack communication
- **Slack Account**: Access to at least one Slack workspace

### Supported Document Types
- **Writer**: Documents, letters, reports (.odt, .docx, .doc, .rtf, .txt)
- **Calc**: Spreadsheets, budgets, data (.ods, .xlsx, .xls, .csv)
- **Impress**: Presentations, slideshows (.odp, .pptx, .ppt)
- **Draw**: Graphics, diagrams, flowcharts (.odg, various image formats)

## How to Share Documents

### Step-by-Step Process

#### 1. Open Your Document
- Launch LibreOffice and open any document
- The document can be new (unsaved) or existing
- Make any final edits before sharing

#### 2. Access Slack Sharing
- Go to the **File** menu
- Select **Share to Slack...**
- The Slack sharing dialog will open

#### 3. First-Time Authentication
If this is your first time using the feature:
- Click **Authenticate** when prompted
- Your web browser will open to Slack's login page
- Sign in to your Slack workspace
- Grant permission to the LibreOffice app
- Return to LibreOffice (authentication completes automatically)

#### 4. Select Destination
- Choose your target from the dropdown menu:
  - **📢 Public channels**: Open to all workspace members
  - **🔒 Private channels**: Restricted membership channels
  - **💬 Direct messages**: 1:1 conversations
  - **👥 Group DMs**: Multi-person private chats

#### 5. Share the Document
- Click the **Share** button
- Wait for upload confirmation
- Your document will appear in the selected Slack destination

### Channel and DM Types

#### Public Channels
- Visible to all workspace members
- Names start with # (e.g., #general, #marketing)
- Anyone in the workspace can join

#### Private Channels  
- Restricted to invited members only
- Show with lock icon 🔒
- Only appear if you're a member

#### Direct Messages
- 1:1 conversations with individual users
- Show user's display name or @username
- Completely private between you and the recipient

#### Group DMs
- Multi-person private conversations
- Show with group icon 👥
- Include multiple participants

## Tips and Best Practices

### Before Sharing
1. **Review Your Document**: Make final edits before sharing
2. **Choose the Right Destination**: Consider who needs access
3. **File Size**: Ensure your document is within Slack's upload limits
4. **Sensitivity**: Use private channels/DMs for confidential content

### Workspace Management
- **Multiple Workspaces**: Re-authenticate when switching workspaces
- **Permissions**: Ensure you have upload permissions in target channels
- **Channel Access**: You can only share to channels you're a member of

### Troubleshooting Common Issues

#### Authentication Problems
**Issue**: "Authentication failed" message
**Solutions**:
- Check your internet connection
- Verify you're logging into the correct Slack workspace
- Ensure your browser allows pop-ups from LibreOffice

#### Channel Not Showing
**Issue**: Expected channel doesn't appear in dropdown
**Solutions**:
- Verify you're a member of the channel
- Check if it's a private channel requiring invitation
- Refresh the channel list by reopening the dialog

#### Upload Failures
**Issue**: "Failed to share file" error
**Solutions**:
- Check your internet connection
- Verify you have upload permissions in the target channel
- Ensure file size is within Slack limits
- Try sharing to a different channel to test

#### Large File Issues
**Issue**: Upload takes too long or fails
**Solutions**:
- Close other applications to free up bandwidth
- Try sharing smaller documents first
- Consider compressing images in your document

## Advanced Features

### Multiple Workspace Support
- You can authenticate with multiple Slack workspaces
- Switch between workspaces by re-authenticating
- Each workspace maintains separate channel lists

### File Format Handling
- Documents are shared in their native LibreOffice format
- Recipients can download and open with LibreOffice or compatible software
- Future updates may include format conversion options

### Batch Operations
- Currently supports one document at a time
- Future versions may support sharing to multiple channels simultaneously

## Privacy and Security

### Data Handling
- Documents are uploaded directly to Slack's servers
- LibreOffice doesn't store copies of your shared documents
- Authentication tokens are stored securely on your local machine

### Permissions
- You can only access channels you're a member of
- Shared documents inherit the privacy settings of the target channel
- Private channels and DMs maintain their restricted access

### Best Practices
1. **Sensitive Documents**: Use private channels or DMs for confidential content
2. **Regular Review**: Periodically review shared documents in Slack
3. **Access Control**: Be mindful of who has access to channels you share to

## Frequently Asked Questions

### General Usage

**Q: Can I share unsaved documents?**
A: Yes, the integration captures the current state of your document, even if unsaved.

**Q: What happens if I edit the document after sharing?**
A: The shared version remains unchanged. You'll need to share again to update.

**Q: Can I share the same document to multiple channels?**
A: Currently, you need to repeat the sharing process for each destination.

### Technical Questions

**Q: Why do I need to authenticate each time I restart LibreOffice?**
A: Authentication tokens are cached between sessions. You should only need to authenticate once per workspace.

**Q: Can I use this without an internet connection?**
A: No, an active internet connection is required for all Slack communication.

**Q: Are there file size limits?**
A: Yes, Slack imposes file size limits (typically 1GB for paid workspaces, smaller for free plans).

### Troubleshooting

**Q: The Share button is grayed out. Why?**
A: Ensure you're authenticated and have selected a valid channel destination.

**Q: I get "not in channel" errors. What does this mean?**
A: You don't have permission to upload to the selected channel. Try a different channel or ask for an invitation.

**Q: The dialog shows "Failed to load channels."**
A: Check your internet connection and re-authenticate if necessary.

## Getting Help

### Built-in Support
- Error messages provide specific guidance for most issues
- The dialog includes help tooltips for major functions

### Technical Support
- Enable debug logging if requested by support
- Document the exact steps that led to any problems
- Note your LibreOffice version and operating system

### Community Resources
- LibreOffice community forums for general questions
- Slack workspace administrators for permission issues
- IT department for enterprise deployment questions

## What's Next

The Slack integration continues to evolve with new features and improvements:

### Planned Enhancements
- **Format Options**: Export to PDF, DOCX, or other formats before sharing
- **Custom Messages**: Add comments or descriptions with shared files
- **Batch Sharing**: Share to multiple destinations simultaneously
- **Template Sharing**: Specialized workflows for document templates

### Feedback
Your feedback helps improve the integration:
- Report bugs or issues you encounter
- Suggest new features that would be valuable
- Share your use cases and workflows

---

**Need immediate help?** Check the troubleshooting section above or consult your IT administrator for workspace-specific guidance.
