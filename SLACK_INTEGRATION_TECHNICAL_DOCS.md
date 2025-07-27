# Slack Integration - Technical Documentation

## Architecture Overview

The Slack integration is implemented as a native LibreOffice feature that leverages modern web APIs to provide seamless document sharing capabilities. The implementation follows LibreOffice's architectural patterns while integrating contemporary OAuth 2.0 and RESTful API standards.

## System Architecture

### Component Diagram
```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   LibreOffice   │    │  OAuth2 Server   │    │  Slack Web API  │
│   File Menu     │    │  (Local HTTPS)   │    │                 │
│        │        │    │                  │    │                 │
│        ▼        │    │                  │    │                 │
│ SlackShareDialog│◄──►│   Port 8081      │◄──►│  oauth.v2.      │
│        │        │    │   (Callback)     │    │  conversations. │
│        ▼        │    │                  │    │  files.         │
│  SlackApiClient │────┼──────────────────┼───►│                 │
│        │        │    │                  │    │                 │
│        ▼        │    │                  │    │                 │
│ Document Stream │    │                  │    │                 │
└─────────────────┘    └──────────────────┘    └─────────────────┘
```

### Core Components

#### 1. User Interface Layer
- **SlackShareDialog**: Main dialog for user interaction
- **UI Definition**: GTK-based interface layout
- **Event Handling**: User actions and feedback

#### 2. API Integration Layer
- **SlackApiClient**: Primary API interface
- **OAuth2Server**: Local authentication server
- **JSON Processing**: Request/response handling

#### 3. Document Processing Layer
- **Stream Management**: Document content capture
- **Temporary File Handling**: Secure document preparation
- **Format Preservation**: Native format maintenance

## Implementation Details

### File Structure
```
sfx2/
├── source/dialog/
│   ├── slackshardialog.cxx         # Main dialog implementation
│   └── slackshardialog.hxx         # Dialog interface
├── uiconfig/ui/
│   └── slackshardialog.ui          # UI layout definition
└── source/doc/
    └── objserv.cxx                 # Menu integration

ucb/source/ucp/slack/
├── SlackApiClient.cxx              # Core API client
├── SlackApiClient.hxx              # API client interface
├── slack_oauth2_server.cxx         # OAuth callback server
├── slack_oauth2_server.hxx         # OAuth server interface
├── slack_json.cxx                  # JSON utilities
├── slack_json.hxx                  # JSON interface
└── slack_datatypes.hxx             # Data structures

config_host/
└── config_oauth2.h                 # OAuth configuration
```

### Key Classes and Methods

#### SlackShareDialog
```cpp
class SlackShareDialog : public weld::GenericDialogController
{
public:
    SlackShareDialog(weld::Window* pParent, 
                     const uno::Reference<io::XInputStream>& xDocumentStream,
                     const OUString& sDocumentName);
    
private:
    void InitializeUI();
    void OnAuthenticateClicked();
    void OnShareClicked();
    void OnChannelSelected();
    void LoadChannels();
    void UpdateShareButtonState();
    
    std::unique_ptr<SlackApiClient> m_pSlackClient;
    std::vector<SlackChannel> m_aChannels;
    // ... UI components
};
```

#### SlackApiClient
```cpp
class SlackApiClient
{
public:
    SlackApiClient();
    ~SlackApiClient();
    
    bool authenticate();
    std::vector<SlackChannel> listChannels(const OUString& workspaceId);
    OUString shareFile(const uno::Reference<io::XInputStream>& stream,
                       const OUString& filename,
                       const OUString& channelId,
                       const OUString& message = OUString());
    
private:
    OUString sendRequestForString(const std::string& method,
                                  const OUString& url,
                                  const std::string& body = "");
    void uploadFileToURL(const OUString& uploadUrl,
                         const uno::Reference<io::XInputStream>& stream);
    
    CURL* m_pCurl;
    OUString m_sAccessToken;
    // ... other members
};
```

### OAuth 2.0 Implementation

#### Flow Diagram
```
1. User clicks "Authenticate"
2. Local HTTPS server starts on port 8081
3. Browser opens Slack OAuth URL
4. User authorizes in Slack
5. Slack redirects to https://localhost:8080/callback
6. HTTPS proxy forwards to http://localhost:8081/callback
7. OAuth server receives authorization code
8. Exchange code for access token
9. Store token for API calls
10. Load user's channels and workspaces
```

#### OAuth Server Implementation
```cpp
class SlackOAuth2Server
{
public:
    SlackOAuth2Server();
    ~SlackOAuth2Server();
    
    bool start(int port = 8081);
    void stop();
    OUString waitForAuthorizationCode(int timeoutSeconds = 120);
    
private:
    void serverLoop();
    OUString parseAuthCodeFromRequest(const std::string& request);
    std::string generateSuccessPage();
    
    int m_serverSocket;
    std::atomic<bool> m_running;
    std::thread m_serverThread;
    OUString m_authCode;
};
```

### API Integration Details

#### Slack API Endpoints Used
1. **oauth.v2.access** - Exchange authorization code for access token
2. **auth.test** - Validate token and get user information
3. **conversations.list** - List available channels and DMs
4. **files.getUploadURLExternal** - Get upload URL for file
5. **files.completeUploadExternal** - Complete file upload process

#### Request/Response Handling
```cpp
// Example API call structure
OUString SlackApiClient::sendRequestForString(const std::string& method,
                                              const OUString& url,
                                              const std::string& body)
{
    // Set up cURL
    curl_easy_setopt(m_pCurl, CURLOPT_URL, url);
    curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, WriteCallback);
    
    // Set headers based on content type
    struct curl_slist* headers = nullptr;
    if (method == "POST") {
        if (url.contains("oauth.v2.access") || url.contains("files.completeUploadExternal")) {
            headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
        } else {
            headers = curl_slist_append(headers, "Content-Type: application/json");
        }
        
        if (!m_sAccessToken.isEmpty()) {
            std::string authHeader = "Authorization: Bearer " + 
                                   OUStringToOString(m_sAccessToken, RTL_TEXTENCODING_UTF8);
            headers = curl_slist_append(headers, authHeader.c_str());
        }
    }
    
    // Execute request and handle response
    CURLcode res = curl_easy_perform(m_pCurl);
    // ... error handling and cleanup
}
```

### Document Stream Handling

#### Current Document Capture
```cpp
// In objserv.cxx - Create document stream for sharing
uno::Reference<io::XInputStream> xDocStream;
sal_Int64 nDocumentSize = 0;

try {
    // Create temporary file to save current document
    uno::Reference<io::XTempFile> xTempFile = 
        io::TempFile::create(comphelper::getProcessComponentContext());
    
    // Get temp file URL
    OUString sTempURL = xTempFile->getUri();
    
    // Save document to temporary file in native format
    uno::Sequence<beans::PropertyValue> aArgs;
    uno::Reference<frame::XStorable> xStorable(GetModel(), uno::UNO_QUERY);
    if (xStorable.is()) {
        xStorable->storeToURL(sTempURL, aArgs);
        
        // Get input stream from temp file
        xDocStream = xTempFile->getInputStream();
        
        // Get size and reset position
        uno::Reference<io::XSeekable> xSeekable(xDocStream, uno::UNO_QUERY);
        if (xSeekable.is()) {
            nDocumentSize = xSeekable->getLength();
            xSeekable->seek(0);
        }
    }
} catch (const uno::Exception& e) {
    // Error handling
}
```

### Security Considerations

#### Token Management
- Access tokens stored in memory only
- No persistent token storage (user must re-authenticate on restart)
- Tokens transmitted over HTTPS only
- OAuth state parameter validation

#### HTTPS Proxy Workaround
Slack requires HTTPS for OAuth callbacks, but LibreOffice runs locally. Solution:
```python
# https_proxy.py - Local HTTPS proxy
import http.server
import ssl
import urllib.request

class ProxyHandler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        # Forward HTTPS requests to local HTTP server
        target_url = f"http://localhost:8081{self.path}"
        response = urllib.request.urlopen(target_url)
        
        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.end_headers()
        self.wfile.write(response.read())

# Create HTTPS server with local certificates
server = http.server.HTTPServer(('localhost', 8080), ProxyHandler)
server.socket = ssl.wrap_socket(server.socket, 
                                certfile='./localhost.pem',
                                keyfile='./localhost-key.pem',
                                server_side=True)
server.serve_forever()
```

## Configuration

### Build Configuration
```makefile
# In relevant .mk files
$(eval $(call gb_Library_use_externals,ucbslack,\
    boost_headers \
    curl \
))

$(eval $(call gb_Library_add_exception_objects,ucbslack,\
    ucb/source/ucp/slack/SlackApiClient \
    ucb/source/ucp/slack/slack_oauth2_server \
    ucb/source/ucp/slack/slack_json \
))
```

### OAuth Configuration
```cpp
// config_host/config_oauth2.h
#define SLACK_CLIENT_ID "your-slack-app-client-id"
#define SLACK_CLIENT_SECRET "your-slack-app-client-secret"
#define SLACK_REDIRECT_URI "https://localhost:8080/callback"
#define SLACK_SCOPES "channels:read,groups:read,im:read,mpim:read,files:write"
```

### Runtime Configuration
```bash
# Debug logging
export SAL_LOG="+WARN.sfx.slack+WARN.ucb.ucp.slack"

# Start HTTPS proxy
python3 https_proxy.py &

# Launch LibreOffice
./instdir/LibreOfficeDev.app/Contents/MacOS/soffice
```

## Error Handling and Logging

### Error Categories
1. **Authentication Errors**: OAuth flow failures, token issues
2. **Network Errors**: Connection timeouts, API unavailability
3. **Permission Errors**: Channel access denied, upload restrictions
4. **Document Errors**: Stream creation failures, file format issues

### Logging Framework
```cpp
// Logging levels and categories
SAL_WARN("sfx.slack", "Authentication failed: " << errorMessage);
SAL_INFO("ucb.ucp.slack", "Channel list loaded: " << channelCount << " channels");
SAL_DEBUG("ucb.ucp.slack", "API request: " << requestUrl);
```

### Error Recovery
- Automatic retry for transient network errors
- Token refresh on authentication failures
- Graceful degradation for channel loading issues
- Clear user feedback for all error conditions

## Performance Considerations

### Optimization Strategies
1. **Asynchronous Operations**: Non-blocking UI during API calls
2. **Connection Reuse**: Single cURL handle for multiple requests
3. **Stream Processing**: Efficient memory usage for large documents
4. **Caching**: Channel lists cached within dialog session

### Memory Management
- RAII patterns for resource cleanup
- Smart pointers for UNO objects
- Proper stream disposal after upload
- cURL handle lifecycle management

## Testing Strategy

### Unit Testing
- Mock Slack API responses
- OAuth flow simulation
- Document stream handling
- JSON parsing validation

### Integration Testing
- Real Slack workspace testing
- Multi-channel upload verification
- Error condition simulation
- Performance benchmarking

### Security Testing
- Token handling validation
- HTTPS proxy security review
- Input sanitization verification
- OAuth flow security assessment

## Deployment Considerations

### Prerequisites
- Modern C++ compiler with C++17 support
- cURL library with SSL support
- Boost libraries for JSON processing
- GTK development libraries for UI

### Platform-Specific Notes

#### macOS
- Code signing requirements for native app
- Keychain integration for secure storage (future)
- App sandbox considerations

#### Linux
- Distribution-specific packaging
- SSL certificate handling variations
- Desktop integration standards

#### Windows
- Certificate store integration
- Windows-specific SSL libraries
- MSI installer considerations

### Enterprise Deployment
- Centralized OAuth app configuration
- Network proxy compatibility
- Security policy compliance
- Administrative controls for feature enablement

## Troubleshooting

### Common Issues and Solutions

#### Build Problems
```bash
# Missing dependencies
sudo apt-get install libcurl4-openssl-dev libboost-all-dev

# Configuration issues
./autogen.sh --with-system-curl --enable-debug
```

#### Runtime Issues
```bash
# SSL certificate problems
mkcert -install
mkcert localhost

# Port conflicts
netstat -an | grep 8081
kill -9 $(lsof -t -i:8081)
```

#### API Integration Issues
```bash
# Token validation
curl -H "Authorization: Bearer $TOKEN" https://slack.com/api/auth.test

# Channel access verification
curl -H "Authorization: Bearer $TOKEN" \
     "https://slack.com/api/conversations.list?types=public_channel,private_channel"
```

## Future Development

### Planned Enhancements
1. **Format Conversion**: PDF, DOCX export options
2. **Batch Operations**: Multi-channel sharing
3. **Advanced Configuration**: Custom OAuth endpoints
4. **Enhanced Security**: Secure token persistence

### Architecture Evolution
- Plugin-based service integration framework
- Unified cloud service authentication
- Cross-platform credential management
- Real-time collaboration features

### API Evolution
- WebSocket support for real-time updates
- Slack Events API integration
- Advanced file metadata handling
- Custom app integration points

---

This technical documentation provides the foundation for maintaining, extending, and troubleshooting the Slack integration. For specific implementation questions, refer to the source code and inline documentation.
