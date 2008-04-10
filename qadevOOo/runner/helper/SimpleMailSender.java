/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SimpleMailSender.java,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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
