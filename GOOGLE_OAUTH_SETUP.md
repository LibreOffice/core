# Google Drive OAuth2 Configuration for LibreOffice

## 🔐 Secure Credential Setup

### Option 1: Environment Variables (Recommended)

Set your credentials as environment variables:

```bash
export GDRIVE_CLIENT_ID="your-client-id-here.apps.googleusercontent.com"
export GDRIVE_CLIENT_SECRET="your-client-secret-here"
```

Then modify the config file to use environment variables:

```cpp
// In config_oauth2.h.in
#define GDRIVE_CLIENT_ID "@GDRIVE_CLIENT_ID@"
#define GDRIVE_CLIENT_SECRET "@GDRIVE_CLIENT_SECRET@"
```

### Option 2: Local Configuration File

Create a local file `config_oauth2_local.h` (git-ignored):

```cpp
#ifndef CONFIG_OAUTH2_LOCAL_H
#define CONFIG_OAUTH2_LOCAL_H

#undef GDRIVE_CLIENT_ID
#undef GDRIVE_CLIENT_SECRET

#define GDRIVE_CLIENT_ID "your-client-id-here.apps.googleusercontent.com"
#define GDRIVE_CLIENT_SECRET "your-client-secret-here"

#endif
```

### Option 3: Build-time Configuration

Add to your configure command:

```bash
./configure \
  --enable-gdrive \
  --with-gdrive-client-id="your-client-id" \
  --with-gdrive-client-secret="your-client-secret"
```

## 🏗️ Google Cloud Console Setup

### 1. Create OAuth2 Credentials

1. Go to [Google Cloud Console](https://console.cloud.google.com/)
2. Select your project or create a new one
3. Navigate to "APIs & Services" > "Credentials"
4. Click "Create Credentials" > "OAuth 2.0 Client IDs"
5. Choose "Desktop application" as application type
6. Set authorized redirect URIs to: `urn:ietf:wg:oauth:2.0:oob`

### 2. Enable Google Drive API

1. Go to "APIs & Services" > "Library"
2. Search for "Google Drive API"
3. Click "Enable"

### 3. Configure OAuth Consent Screen

1. Go to "APIs & Services" > "OAuth consent screen"
2. Choose "External" user type
3. Fill in required fields:
   - App name: "LibreOffice Google Drive Integration"
   - User support email: your email
   - Developer contact: your email
4. Add scopes: `https://www.googleapis.com/auth/drive.file`

## 🔧 Integration with GoogleDriveApiClient

The credentials will be automatically used in the OAuth2 flow:

```cpp
// In GoogleDriveApiClient.cxx - OAuth2 URL construction
rtl::OUString authUrl = rtl::OUString::createFromAscii(
    "https://accounts.google.com/o/oauth2/v2/auth"
    "?response_type=code"
    "&client_id=" GDRIVE_CLIENT_ID
    "&redirect_uri=" GDRIVE_REDIRECT_URI  
    "&scope=" GDRIVE_SCOPE
);
```

## ⚠️ Security Notes

- **NEVER** commit actual credentials to version control
- Use environment variables or local config files
- Add `config_oauth2_local.h` to `.gitignore`
- Use least-privilege scopes (we use `drive.file` not `drive`)

## 🧪 Testing OAuth2 Flow

After configuration, test the authentication:

1. Start LibreOffice
2. File → Open → Google Drive
3. Should open browser for Google OAuth consent
4. Grant permissions
5. Copy authorization code back to LibreOffice
