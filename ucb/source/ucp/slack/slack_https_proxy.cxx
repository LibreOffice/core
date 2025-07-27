/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/**
 * Simple HTTPS proxy executable for Slack OAuth2 callback
 * This is a standalone executable that can be bundled with LibreOffice
 * and started automatically when needed for OAuth flows
 */

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <cstdlib>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/wait.h>
#endif

int main(int argc, char* argv[])
{
    std::cout << "LibreOffice HTTPS Proxy for Slack OAuth" << std::endl;

    // Check if mkcert certificates exist
    bool certExists = false;

    // Try different certificate locations
    std::vector<std::string> certPaths = {
        "localhost+1.pem",
        "ucb/source/ucp/slack/certs/localhost+1.pem",
        std::string(getenv("HOME") ? getenv("HOME") : "") + "/.local/share/mkcert/localhost+1.pem"
    };

    std::string certFile, keyFile;
    for (const auto& path : certPaths) {
        std::string keyPath = path;
        size_t dotPos = keyPath.find(".pem");
        if (dotPos != std::string::npos) {
            keyPath.replace(dotPos, 4, "-key.pem");
        }

        // Check if both files exist
        FILE* cert = fopen(path.c_str(), "r");
        FILE* key = fopen(keyPath.c_str(), "r");

        if (cert && key) {
            certFile = path;
            keyFile = keyPath;
            certExists = true;
            fclose(cert);
            fclose(key);
            break;
        }

        if (cert) fclose(cert);
        if (key) fclose(key);
    }

    if (!certExists) {
        std::cerr << "SSL certificates not found. Running certificate setup..." << std::endl;

        // Try to run the setup script
        int result = system("./setup_ssl_certificates.sh");
        if (result != 0) {
            std::cerr << "Failed to setup SSL certificates. Please run: ./setup_ssl_certificates.sh" << std::endl;
            return 1;
        }

        // Check again
        certFile = "localhost+1.pem";
        keyFile = "localhost+1-key.pem";
        FILE* cert = fopen(certFile.c_str(), "r");
        FILE* key = fopen(keyFile.c_str(), "r");

        if (!cert || !key) {
            std::cerr << "SSL certificates still not found after setup" << std::endl;
            if (cert) fclose(cert);
            if (key) fclose(key);
            return 1;
        }

        fclose(cert);
        fclose(key);
    }

    std::cout << "Using SSL certificate: " << certFile << std::endl;
    std::cout << "Using SSL key: " << keyFile << std::endl;

    // Start the Python HTTPS proxy with found certificates
    std::string command = "python3 -c \""
        "import http.server, socketserver, ssl, urllib.request, urllib.parse, threading, os, sys; "
        "class HTTPSProxyHandler(http.server.BaseHTTPRequestHandler): "
        "    def do_GET(self): "
        "        try: "
        "            target_url = f'http://localhost:8081{self.path}'; "
        "            with urllib.request.urlopen(target_url) as response: "
        "                content = response.read(); "
        "            self.send_response(200); "
        "            self.send_header('Content-type', 'text/html'); "
        "            self.end_headers(); "
        "            self.wfile.write(content); "
        "        except Exception as e: "
        "            print(f'Error forwarding request: {e}'); "
        "            self.send_response(200); "
        "            self.send_header('Content-type', 'text/html'); "
        "            self.end_headers(); "
        "            self.wfile.write(b'<html><body><h1>OAuth Callback Received</h1><p>You can close this window.</p></body></html>'); "
        "httpd = socketserver.TCPServer(('', 8080), HTTPSProxyHandler); "
        "context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER); "
        "context.load_cert_chain('" + certFile + "', '" + keyFile + "'); "
        "httpd.socket = context.wrap_socket(httpd.socket, server_side=True); "
        "print('HTTPS proxy running on https://localhost:8080'); "
        "print('Forwarding to http://localhost:8081'); "
        "try: httpd.serve_forever(); "
        "except KeyboardInterrupt: print('Shutting down...')\"";

    std::cout << "Starting HTTPS proxy..." << std::endl;
    std::cout << "Press Ctrl+C to stop" << std::endl;

    // Execute the proxy
    int result = system(command.c_str());

    std::cout << "HTTPS proxy stopped" << std::endl;
    return result;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
