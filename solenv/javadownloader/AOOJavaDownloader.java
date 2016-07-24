/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.IOException;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLConnection;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

public class AOOJavaDownloader {
    private static final int MAX_REDIRECTS = 10;
    private static final String hexChars = "0123456789abcdef";

    public static void main(String[] args) throws MalformedURLException, NoSuchAlgorithmException {
        if (args.length != 2 && args.length != 4) {
            System.err.println("AOOJavaDownloader - required args: filename URL [checksumType] [checksum]");
            System.exit(1);
        }
        String name = args[0];
        URL url = new URL(args[1]);
        String checksumType = args.length == 4 ? args[2] : null;
        String checksum = args.length == 4 ? args[3] : null;

        File filename = new File(System.getenv("TARFILE_LOCATION"), name);

        MessageDigest digest = null;
        if (checksumType != null && checksumType.equals("SHA1")) {
            digest = MessageDigest.getInstance(checksumType);
        } else {
            digest = MessageDigest.getInstance("MD5");
        }

        File temporaryFile = new File(filename.getAbsolutePath() + ".part");
        System.out.println("downloading " + url + " to " + temporaryFile.getAbsolutePath());

        boolean succeeded = false;
        InputStream inputStream = null;
        try {
            inputStream = openURL(url);

            OutputStream outputStream = new BufferedOutputStream(new FileOutputStream(temporaryFile));
            try {
                copyStreamAndHash(inputStream, outputStream, digest);
            } finally {
                try {
                    outputStream.close();
                } catch (IOException ignored) {
                }
            }

            String fileChecksum = bytesToHex(digest.digest());
            if (checksumType != null) {
                if (fileChecksum.equals(checksum)) {
                    System.out.println(checksumType + " checksum is OK");
                } else {
                    temporaryFile.delete();
                    throw new IOException(String.format("%s checksum does not match (%s instead of %s)",
                            checksumType, fileChecksum, checksum));
                }
            } else {
                System.out.println("checksum not given, md5 of file is " + fileChecksum);
                filename = new File(System.getenv("TARFILE_LOCATION"), fileChecksum + "-" + name);
            }
            if (!temporaryFile.renameTo(filename)) {
                throw new IOException("Renaming " + temporaryFile + " to " + filename + " failed");
            }
            succeeded = true;
        } catch (Exception exception) {
            System.out.println("download failed:");
            exception.printStackTrace(System.out);
        } finally {
            if (inputStream != null) {
                try {
                    inputStream.close();
                } catch (IOException ignored) {
                }
            }
            if (!succeeded) {
                temporaryFile.delete();
            }
        }
        System.exit(succeeded ? 0 : 1);
    }

    private static InputStream openURL(URL url) throws IOException {
        URLConnection connection = url.openConnection();
        for (int redir = 0; redir < MAX_REDIRECTS; redir++) {
            connection.setAllowUserInteraction(false);
            connection.setConnectTimeout(60000);
            connection.setReadTimeout(60000);
            if (connection instanceof HttpURLConnection) {
                HttpURLConnection httpURLConnection = (HttpURLConnection) connection;
                // Only works when protocol (http:// or https://) remains the same:
                httpURLConnection.setInstanceFollowRedirects(true);
            }
            connection.connect();
            if (connection instanceof HttpURLConnection) {
                HttpURLConnection httpURLConnection = (HttpURLConnection) connection;
                // handle protocol change:
                int status = httpURLConnection.getResponseCode();
                if (status == HttpURLConnection.HTTP_MOVED_TEMP ||
                        status == HttpURLConnection.HTTP_MOVED_PERM ||
                        status == HttpURLConnection.HTTP_SEE_OTHER) {
                    connection = new URL(connection.getHeaderField("Location")).openConnection();
                    continue;
                }
            }
            return new BufferedInputStream(connection.getInputStream());
        }
        throw new IOException("Too many http redirects");
    }

    private static void copyStreamAndHash(InputStream inputStream, OutputStream outputStream, MessageDigest digest) throws IOException {
        byte[] buffer = new byte[4 * 4096];
        int bytesRead;
        while ((bytesRead = inputStream.read(buffer)) >= 0) {
            outputStream.write(buffer, 0, bytesRead);
            digest.update(buffer, 0, bytesRead);
        }
    }

    private static String bytesToHex(byte[] bytes) {
        char[] output = new char[bytes.length * 2];
        for (int i = 0; i < bytes.length; i++) {
            output[2*i] = hexChars.charAt(0xf & (bytes[i] >> 4));
            output[2*i + 1] = hexChars.charAt(0xf & bytes[i]);
        }
        return new String(output);
    }
}
