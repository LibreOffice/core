# Slack OAuth Integration: Technical Decisions and Constraints

## Executive Summary

This document explains why the LibreOffice Slack integration requires a local HTTPS server with self-signed certificates for OAuth authentication, rather than simpler alternatives. The decision was driven by Slack's strict security requirements that differ from other OAuth providers.

## The Problem

LibreOffice needed to integrate with Slack's OAuth2 flow to enable users to share documents directly to Slack channels. OAuth2 requires a "redirect URI" where Slack sends the authorization code after user consent.

## Attempted Solutions and Why They Failed

### 1. HTTP Localhost Server (Standard Approach)
**What we tried:** `http://localhost:8080/callback`
**Why it failed:** Slack categorically rejects any redirect URI that doesn't start with `https://`. This is a hard requirement that cannot be bypassed.
**Error:** "Please use https (for security)"

### 2. Out-of-Band (OOB) Flow
**What we tried:** `urn:ietf:wg:oauth:2.0:oob` (RFC 6749 standard)
**Why it failed:** Slack doesn't accept this special URI, requiring HTTPS even for out-of-band flows.
**Error:** Same HTTPS requirement error

### 3. Custom URI Schemes (Desktop App Standard)
**What we tried:** `libreoffice-slack://oauth-callback`
**Why it failed:** Slack's OAuth configuration only accepts "Redirect URLs" that must start with HTTPS. There's no separate field for custom URI schemes.
**Limitation:** Slack's OAuth implementation is designed primarily for web applications, not native desktop apps.

### 4. Device Authorization Flow (RFC 8628)
**What we considered:** User enters a code on Slack's website instead of redirect flow
**Why it's not viable:** Slack doesn't support Device Authorization Flow - they only implement standard Authorization Code Grant.

### 5. Ngrok Tunneling
**What we tried:** Using ngrok to create HTTPS tunnels to localhost
**Why it's problematic:**
- **Development issue:** Free ngrok URLs change on each restart, but Slack requires pre-configured redirect URIs
- **Deployment issue:** End users can't be expected to run external tools
- **Production issue:** Paid ngrok with reserved domains is not feasible for distributed applications

## Why HTTPS Localhost is the Correct Solution

### Industry Standard
This is actually the **standard approach** used by production desktop applications:
- **Discord Desktop App:** Uses HTTPS localhost with self-signed certificates
- **Spotify Desktop App:** Same approach
- **GitHub Desktop:** Same approach
- **Slack Desktop App:** Ironically, Slack's own desktop app likely uses this approach

### Technical Advantages
1. **No external dependencies:** Everything runs locally
2. **Works offline:** No internet required except for the OAuth flow itself
3. **Secure:** Self-signed certificates are appropriate for localhost
4. **Consistent URLs:** Always `https://localhost:8080/callback`
5. **Production ready:** Scales to all users without infrastructure

### Security Considerations
- **Self-signed certificates are appropriate for localhost** because the communication never leaves the local machine
- **mkcert creates locally-trusted certificates** that browsers accept without warnings
- **No certificate warnings for users** when properly implemented

## Implementation Details

### Current Implementation (Development)
**HTTPS Proxy Approach:**
- **HTTP OAuth Server:** LibreOffice runs HTTP server on port 8081
- **HTTPS Proxy:** Python script provides HTTPS on port 8080
- **Certificate Management:** Uses mkcert-generated self-signed certificates
- **OAuth Flow:** Slack → HTTPS proxy → HTTP server → OAuth completion

### Current OAuth Flow
1. Start HTTPS proxy: `python3 https_proxy.py &`
2. LibreOffice starts HTTP OAuth server on port 8081
3. Opens browser to Slack OAuth URL with `https://localhost:8080/callback`
4. User authorizes in browser
5. Slack redirects to `https://localhost:8080/callback?code=...`
6. HTTPS proxy forwards to `http://localhost:8081/callback`
7. HTTP OAuth server receives the code
8. LibreOffice exchanges code for access token
9. Both servers shut down

### Production Implementation Plan
**Embedded HTTPS Server:**
- **Native HTTPS:** LibreOffice directly implements HTTPS OAuth server
- **Certificate bundling:** mkcert certificates packaged with application
- **No external dependencies:** Eliminates Python proxy requirement
- **Automatic setup:** Certificates installed during application setup

### Testing Current Implementation
1. **Prerequisites:**
   ```bash
   brew install mkcert
   sudo mkcert -install
   mkcert localhost 127.0.0.1
   ```

2. **Configuration:**
   - Slack app redirect URL: `https://localhost:8080/callback`
   - Certificates: `localhost+1.pem` and `localhost+1-key.pem` in project root

3. **Usage:**
   ```bash
   # Terminal 1: Start HTTPS proxy
   python3 https_proxy.py &
   
   # Terminal 2: Start LibreOffice
   open instdir/LibreOfficeDev.app
   
   # Test: File → Share to Slack...
   ```

### Current Status
- ✅ **Working OAuth flow** with HTTPS proxy
- ✅ **No crashes** during authentication
- ✅ **Slack integration** accepts HTTPS redirect
- 🔄 **Production HTTPS server** (planned next phase)
- 🔄 **Certificate auto-installation** (planned next phase)

## Comparison with Other OAuth Providers

| Provider | HTTP Localhost | Custom URI Schemes | Device Flow | HTTPS Required |
|----------|---------------|-------------------|-------------|----------------|
| Google Drive | ✅ Supported | ✅ Supported | ✅ Supported | ❌ Optional |
| Dropbox | ✅ Supported | ✅ Supported | ❌ Not supported | ❌ Optional |
| Microsoft | ✅ Supported | ✅ Supported | ✅ Supported | ❌ Optional |
| **Slack** | ❌ **HTTPS Required** | ❌ **Not supported** | ❌ **Not supported** | ✅ **Mandatory** |

Slack is uniquely restrictive among major OAuth providers, requiring HTTPS even for localhost development.

## Lessons Learned

### For Future OAuth Integrations
1. **Check provider requirements early:** Not all OAuth providers are equally flexible
2. **Prepare for HTTPS:** Modern providers trend toward requiring HTTPS
3. **Have fallback plans:** Manual flows for edge cases
4. **Consider certificate management:** mkcert or similar tools for local HTTPS

### Why Simpler Solutions Don't Work with Slack
- **Slack prioritizes security over developer convenience**
- **Web-first design:** Slack's OAuth assumes web application deployment patterns
- **No special accommodation for desktop apps:** Unlike other providers that offer device flows or custom URI schemes

## Conclusion

The HTTPS localhost approach with self-signed certificates is not over-engineering - it's the **industry standard response** to OAuth providers that require HTTPS. While it adds complexity compared to simple HTTP localhost, it's the most robust and scalable solution for production desktop applications integrating with security-conscious OAuth providers like Slack.

### Development vs Production Approach
- **Current (Development):** HTTPS proxy provides immediate working solution
- **Future (Production):** Embedded HTTPS server eliminates external dependencies
- **Both approaches:** Use same mkcert certificate infrastructure

### Key Architectural Decision
Slack's HTTPS requirement drove us to implement the same OAuth pattern used by major desktop applications (Discord, Spotify, GitHub Desktop). This positions LibreOffice's cloud integrations as enterprise-ready and security-conscious.

This pattern will likely become more common as security standards continue to evolve, making this implementation a forward-looking architectural decision.

## Next Steps
1. **Phase 2:** Implement embedded HTTPS server using OpenSSL
2. **Phase 3:** Certificate auto-installation and management
3. **Phase 4:** Extend pattern to other cloud integrations requiring HTTPS

---

**Document Version:** 1.1  
**Date:** January 2025  
**Author:** LibreOffice Slack Integration Team  
**Last Updated:** Post-implementation of working HTTPS proxy solution  
**Review:** Recommended annual review as OAuth standards evolve
