/*************************************************************************
 *
 *  $RCSfile: SimpleMailSender.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2004-12-10 17:00:35 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed input the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except input compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
