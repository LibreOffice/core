/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
package helper;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;

import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;


public class SimpleMailSender {
    private String m_MailServer = "unknown";
    private String m_Sender = "unknown";
    private String m_Recipient = "unknown";
    private String m_Subject = "unknown";
    private String m_Message = "unknown";


    public String getMailServer() {
        return m_MailServer;
    }

    public void setMailServer(String server) {
        m_MailServer = server;
    }

    public String getSender() {
        return m_Sender;
    }

    public void setSender(String sender) {
        m_Sender = sender;
    }

    public String getRecipient() {
        return m_Recipient;
    }

    public void setRecipient(String recipient) {
        m_Recipient = recipient;
    }

    public String getSubject() {
        return m_Subject;
    }

    public void setSubject(String subject) {
        m_Subject = subject;
    }

    public String getMessage() {
        return m_Message;
    }

    public void setMessage(String msg) {
        m_Message = msg;
    }

    public void sendMail() {
        if (m_MailServer.equals ("unknown")) {
            System.out.println("No Mailserver given ... exiting");
            return;
        }
        if (m_Recipient.equals ("unknown")) {
            System.out.println("No Recipient given ... exiting");
            return;
        }
        sendMail(m_MailServer,m_Sender,m_Recipient,m_Subject,m_Message);
    }

    public void sendMail(String server, String sender,
        String recipient, String subject, String msg) {
        //setting member variables for reuse
        m_MailServer = server;
        m_Sender = sender;
        m_Recipient = recipient;
        m_Subject = subject;
        m_Message = msg;

        try {
            Socket socket = new Socket(m_MailServer, 25);
            BufferedReader input =
                new BufferedReader(new InputStreamReader(
                        socket.getInputStream(), "8859_1"));
            BufferedWriter output =
                new BufferedWriter(new OutputStreamWriter(
                        socket.getOutputStream(), "8859_1"));

            sendline(input, output, "HELO " + getHostName());
            sendline(input, output, "MAIL FROM: " + m_Sender);
            sendline(input, output, "RCPT TO: <" + m_Recipient + ">");
            sendline(input, output, "DATA");
            sendline(output, "MIME-Version: 1.0");
            sendline(output, "Subject: " + m_Subject);
            sendline(output, "From: " + m_Sender);
            sendline(output, "To: " + m_Recipient);
            sendline(output,
                "Content-Type: text/html; charset=\"us-ascii\"\r\n");

            // Send the body
            sendline(output, m_Message);

            sendline(input, output, ".");
            sendline(input, output, "QUIT");
            socket.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void sendline(BufferedReader input,
        BufferedWriter output, String line) {
        try {
            output.write(line + "\r\n");
            output.flush();
            line = input.readLine();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void sendline(BufferedWriter output, String line) {
        try {
            output.write(line + "\r\n");
            output.flush();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private String getHostName() {
        String hostname = "";

        try {
            InetAddress addr = InetAddress.getLocalHost();

            hostname = addr.getHostName();
        } catch (UnknownHostException e) {
        }

        return hostname;
    }
}
