# *************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
# *************************************************************

# Caolan McNamara caolanm@redhat.com
# a simple email mailmerge component

# manual installation for hackers, not necessary for users
# cp mailmerge.py /usr/lib/openoffice.org2.0/program
# cd /usr/lib/openoffice.org2.0/program
# ./unopkg add --shared mailmerge.py
# edit ~/.openoffice.org2/user/registry/data/org/openoffice/Office/Writer.xcu
# and change EMailSupported to as follows...
#  <prop oor:name="EMailSupported" oor:type="xs:boolean">
#   <value>true</value>
#  </prop>

import unohelper
import uno
import re

#to implement com::sun::star::mail::XMailServiceProvider
#and
#to implement com.sun.star.mail.XMailMessage

from com.sun.star.mail import XMailServiceProvider
from com.sun.star.mail import XMailService
from com.sun.star.mail import XSmtpService
from com.sun.star.mail import XConnectionListener
from com.sun.star.mail import XAuthenticator
from com.sun.star.mail import XMailMessage
from com.sun.star.mail.MailServiceType import SMTP
from com.sun.star.mail.MailServiceType import POP3
from com.sun.star.mail.MailServiceType import IMAP
from com.sun.star.uno import XCurrentContext
from com.sun.star.lang import IllegalArgumentException
from com.sun.star.lang import EventObject
from com.sun.star.mail import SendMailMessageFailedException

from email.MIMEBase import MIMEBase
from email.Message import Message
from email import Encoders
from email.Header import Header
from email.MIMEMultipart import MIMEMultipart
from email.Utils import formatdate
from email.Utils import parseaddr
from socket import _GLOBAL_DEFAULT_TIMEOUT

import sys, smtplib, imaplib, poplib

dbg = False

class PyMailSMTPService(unohelper.Base, XSmtpService):
    def __init__( self, ctx ):
        self.ctx = ctx
        self.listeners = []
        self.supportedtypes = ('Insecure', 'Ssl')
        self.server = None
        self.connectioncontext = None
        self.notify = EventObject(self)
        if dbg:
            print("PyMailSMPTService init", file=sys.stderr)
    def addConnectionListener(self, xListener):
        if dbg:
            print("PyMailSMPTService addConnectionListener", file=sys.stderr)
        self.listeners.append(xListener)
    def removeConnectionListener(self, xListener):
        if dbg:
            print("PyMailSMPTService removeConnectionListener", file=sys.stderr)
        self.listeners.remove(xListener)
    def getSupportedConnectionTypes(self):
        if dbg:
            print("PyMailSMPTService getSupportedConnectionTypes", file=sys.stderr)
        return self.supportedtypes
    def connect(self, xConnectionContext, xAuthenticator):
        self.connectioncontext = xConnectionContext
        if dbg:
            print("PyMailSMPTService connect", file=sys.stderr)

        server = xConnectionContext.getValueByName("ServerName")
        if dbg:
            print("ServerName: %s" % server, file=sys.stderr)

        port = xConnectionContext.getValueByName("Port")
        if dbg:
            print("Port: %d" % port, file=sys.stderr)

        tout = xConnectionContext.getValueByName("Timeout")
        if dbg:
            print(isinstance(tout,int), file=sys.stderr)
        if not isinstance(tout,int):
            tout = _GLOBAL_DEFAULT_TIMEOUT
        if dbg:
            print("Timeout: %s" % str(tout), file=sys.stderr)

        self.server = smtplib.SMTP(server, port,timeout=tout)
        if dbg:
            self.server.set_debuglevel(1)

        connectiontype = xConnectionContext.getValueByName("ConnectionType")
        if dbg:
            print("ConnectionType: %s" % connectiontype, file=sys.stderr)

        if connectiontype.upper() == 'SSL':
            self.server.ehlo()
            self.server.starttls()
            self.server.ehlo()

        user = xAuthenticator.getUserName().encode('ascii')
        password = xAuthenticator.getPassword().encode('ascii')
        if user != '':
            if dbg:
                print('Logging in, username of', user, file=sys.stderr)
            self.server.login(user, password)

        for listener in self.listeners:
            listener.connected(self.notify)
    def disconnect(self):
        if dbg:
            print("PyMailSMPTService disconnect", file=sys.stderr)
        if self.server:
            self.server.quit()
            self.server = None
        for listener in self.listeners:
            listener.disconnected(self.notify)
    def isConnected(self):
        if dbg:
            print("PyMailSMPTService isConnected", file=sys.stderr)
        return self.server != None
    def getCurrentConnectionContext(self):
        if dbg:
            print("PyMailSMPTService getCurrentConnectionContext", file=sys.stderr)
        return self.connectioncontext
    def sendMailMessage(self, xMailMessage):
        COMMASPACE = ', '

        if dbg:
            print("PyMailSMPTService sendMailMessage", file=sys.stderr)
        recipients = xMailMessage.getRecipients()
        sendermail = xMailMessage.SenderAddress
        sendername = xMailMessage.SenderName
        subject = xMailMessage.Subject
        ccrecipients = xMailMessage.getCcRecipients()
        bccrecipients = xMailMessage.getBccRecipients()
        if dbg:
            print("PyMailSMPTService subject", subject, file=sys.stderr)
            print("PyMailSMPTService from", sendername.encode('utf-8'), file=sys.stderr)
            print("PyMailSMTPService from", sendermail, file=sys.stderr)
            print("PyMailSMPTService send to", recipients, file=sys.stderr)

        attachments = xMailMessage.getAttachments()

        textmsg = Message()

        content = xMailMessage.Body
        flavors = content.getTransferDataFlavors()
        if dbg:
            print("PyMailSMPTService flavors len", len(flavors), file=sys.stderr)

        #Use first flavor that's sane for an email body
        for flavor in flavors:
            if flavor.MimeType.find('text/html') != -1 or flavor.MimeType.find('text/plain') != -1:
                if dbg:
                    print("PyMailSMPTService mimetype is", flavor.MimeType, file=sys.stderr)
                textbody = content.getTransferData(flavor)
                try:
                    textbody = textbody.value
                except:
                    pass
                textbody = textbody.encode('utf-8')

                if len(textbody):
                    mimeEncoding = re.sub("charset=.*", "charset=UTF-8", flavor.MimeType)
                    if mimeEncoding.find('charset=UTF-8') == -1:
                        mimeEncoding = mimeEncoding + "; charset=UTF-8"
                    textmsg['Content-Type'] = mimeEncoding
                    textmsg['MIME-Version'] = '1.0'
                    textmsg.set_payload(textbody)

                break

        if (len(attachments)):
            msg = MIMEMultipart()
            msg.epilogue = ''
            msg.attach(textmsg)
        else:
            msg = textmsg

        hdr = Header(sendername, 'utf-8')
        hdr.append('<'+sendermail+'>','us-ascii')
        msg['Subject'] = subject
        msg['From'] = hdr
        msg['To'] = COMMASPACE.join(recipients)
        if len(ccrecipients):
            msg['Cc'] = COMMASPACE.join(ccrecipients)
        if xMailMessage.ReplyToAddress != '':
            msg['Reply-To'] = xMailMessage.ReplyToAddress

        mailerstring = "OpenOffice.org 2.0 via Caolan's mailmerge component"
        try:
            ctx = uno.getComponentContext()
            aConfigProvider = ctx.ServiceManager.createInstance("com.sun.star.configuration.ConfigurationProvider")
            prop = uno.createUnoStruct('com.sun.star.beans.PropertyValue')
            prop.Name = "nodepath"
            prop.Value = "/org.openoffice.Setup/Product"
            aSettings = aConfigProvider.createInstanceWithArguments("com.sun.star.configuration.ConfigurationAccess",
                    (prop,))
            mailerstring = aSettings.getByName("ooName") + " " + \
                    aSettings.getByName("ooSetupVersion") + " via Caolan's mailmerge component"
        except:
            pass

        msg['X-Mailer'] = mailerstring
        msg['Date'] = formatdate(localtime=True)

        for attachment in attachments:
            content = attachment.Data
            flavors = content.getTransferDataFlavors()
            flavor = flavors[0]
            ctype = flavor.MimeType
            maintype, subtype = ctype.split('/', 1)
            msgattachment = MIMEBase(maintype, subtype)
            data = content.getTransferData(flavor)
            msgattachment.set_payload(data)
            Encoders.encode_base64(msgattachment)
            fname = attachment.ReadableName
            try:
                fname.encode('ascii')
            except:
                fname = ('utf-8','',fname.encode('utf-8'))
            msgattachment.add_header('Content-Disposition', 'attachment', \
                    filename=fname)
            msg.attach(msgattachment)

        uniquer = {}
        for key in recipients:
            uniquer[key] = True
        if len(ccrecipients):
            for key in ccrecipients:
                uniquer[key] = True
        if len(bccrecipients):
            for key in bccrecipients:
                uniquer[key] = True
        truerecipients = list(uniquer.keys())

        if dbg:
            print("PyMailSMPTService recipients are", truerecipients, file=sys.stderr)

        self.server.sendmail(sendermail, truerecipients, msg.as_string())

class PyMailIMAPService(unohelper.Base, XMailService):
    def __init__( self, ctx ):
        self.ctx = ctx
        self.listeners = []
        self.supportedtypes = ('Insecure', 'Ssl')
        self.server = None
        self.connectioncontext = None
        self.notify = EventObject(self)
        if dbg:
            print("PyMailIMAPService init", file=sys.stderr)
    def addConnectionListener(self, xListener):
        if dbg:
            print("PyMailIMAPService addConnectionListener", file=sys.stderr)
        self.listeners.append(xListener)
    def removeConnectionListener(self, xListener):
        if dbg:
            print("PyMailIMAPService removeConnectionListener", file=sys.stderr)
        self.listeners.remove(xListener)
    def getSupportedConnectionTypes(self):
        if dbg:
            print("PyMailIMAPService getSupportedConnectionTypes", file=sys.stderr)
        return self.supportedtypes
    def connect(self, xConnectionContext, xAuthenticator):
        if dbg:
            print("PyMailIMAPService connect", file=sys.stderr)

        self.connectioncontext = xConnectionContext
        server = xConnectionContext.getValueByName("ServerName")
        if dbg:
            print(server, file=sys.stderr)
        port = xConnectionContext.getValueByName("Port")
        if dbg:
            print(port, file=sys.stderr)
        connectiontype = xConnectionContext.getValueByName("ConnectionType")
        if dbg:
            print(connectiontype, file=sys.stderr)
        print("BEFORE", file=sys.stderr)
        if connectiontype.upper() == 'SSL':
            self.server = imaplib.IMAP4_SSL(server, port)
        else:
            self.server = imaplib.IMAP4(server, port)
        print("AFTER", file=sys.stderr)

        user = xAuthenticator.getUserName().encode('ascii')
        password = xAuthenticator.getPassword().encode('ascii')
        if user != '':
            if dbg:
                print('Logging in, username of', user, file=sys.stderr)
            self.server.login(user, password)

        for listener in self.listeners:
            listener.connected(self.notify)
    def disconnect(self):
        if dbg:
            print("PyMailIMAPService disconnect", file=sys.stderr)
        if self.server:
            self.server.logout()
            self.server = None
        for listener in self.listeners:
            listener.disconnected(self.notify)
    def isConnected(self):
        if dbg:
            print("PyMailIMAPService isConnected", file=sys.stderr)
        return self.server != None
    def getCurrentConnectionContext(self):
        if dbg:
            print("PyMailIMAPService getCurrentConnectionContext", file=sys.stderr)
        return self.connectioncontext

class PyMailPOP3Service(unohelper.Base, XMailService):
    def __init__( self, ctx ):
        self.ctx = ctx
        self.listeners = []
        self.supportedtypes = ('Insecure', 'Ssl')
        self.server = None
        self.connectioncontext = None
        self.notify = EventObject(self)
        if dbg:
            print("PyMailPOP3Service init", file=sys.stderr)
    def addConnectionListener(self, xListener):
        if dbg:
            print("PyMailPOP3Service addConnectionListener", file=sys.stderr)
        self.listeners.append(xListener)
    def removeConnectionListener(self, xListener):
        if dbg:
            print("PyMailPOP3Service removeConnectionListener", file=sys.stderr)
        self.listeners.remove(xListener)
    def getSupportedConnectionTypes(self):
        if dbg:
            print("PyMailPOP3Service getSupportedConnectionTypes", file=sys.stderr)
        return self.supportedtypes
    def connect(self, xConnectionContext, xAuthenticator):
        if dbg:
            print("PyMailPOP3Service connect", file=sys.stderr)

        self.connectioncontext = xConnectionContext
        server = xConnectionContext.getValueByName("ServerName")
        if dbg:
            print(server, file=sys.stderr)
        port = xConnectionContext.getValueByName("Port")
        if dbg:
            print(port, file=sys.stderr)
        connectiontype = xConnectionContext.getValueByName("ConnectionType")
        if dbg:
            print(connectiontype, file=sys.stderr)
        print("BEFORE", file=sys.stderr)
        if connectiontype.upper() == 'SSL':
            self.server = poplib.POP3_SSL(server, port)
        else:
            tout = xConnectionContext.getValueByName("Timeout")
            if dbg:
                print(isinstance(tout,int), file=sys.stderr)
            if not isinstance(tout,int):
                tout = _GLOBAL_DEFAULT_TIMEOUT
            if dbg:
                print("Timeout: %s" % str(tout), file=sys.stderr)
            self.server = poplib.POP3(server, port, timeout=tout)
        print("AFTER", file=sys.stderr)

        user = xAuthenticator.getUserName().encode('ascii')
        password = xAuthenticator.getPassword().encode('ascii')
        if dbg:
            print('Logging in, username of', user, file=sys.stderr)
        self.server.user(user)
        self.server.pass_(password)

        for listener in self.listeners:
            listener.connected(self.notify)
    def disconnect(self):
        if dbg:
            print("PyMailPOP3Service disconnect", file=sys.stderr)
        if self.server:
            self.server.quit()
            self.server = None
        for listener in self.listeners:
            listener.disconnected(self.notify)
    def isConnected(self):
        if dbg:
            print("PyMailPOP3Service isConnected", file=sys.stderr)
        return self.server != None
    def getCurrentConnectionContext(self):
        if dbg:
            print("PyMailPOP3Service getCurrentConnectionContext", file=sys.stderr)
        return self.connectioncontext

class PyMailServiceProvider(unohelper.Base, XMailServiceProvider):
    def __init__( self, ctx ):
        if dbg:
            print("PyMailServiceProvider init", file=sys.stderr)
        self.ctx = ctx
    def create(self, aType):
        if dbg:
            print("PyMailServiceProvider create with", aType, file=sys.stderr)
        if aType == SMTP:
            return PyMailSMTPService(self.ctx);
        elif aType == POP3:
            return PyMailPOP3Service(self.ctx);
        elif aType == IMAP:
            return PyMailIMAPService(self.ctx);
        else:
            print("PyMailServiceProvider, unknown TYPE", aType, file=sys.stderr)

class PyMailMessage(unohelper.Base, XMailMessage):
    def __init__( self, ctx, sTo='', sFrom='', Subject='', Body=None, aMailAttachment=None ):
        if dbg:
            print("PyMailMessage init", file=sys.stderr)
        self.ctx = ctx

        self.recipients = [sTo]
        self.ccrecipients = []
        self.bccrecipients = []
        self.aMailAttachments = []
        if aMailAttachment != None:
            self.aMailAttachments.append(aMailAttachment)

        self.SenderName, self.SenderAddress = parseaddr(sFrom)
        self.ReplyToAddress = sFrom
        self.Subject = Subject
        self.Body = Body
        if dbg:
            print("post PyMailMessage init", file=sys.stderr)
    def addRecipient( self, recipient ):
        if dbg:
            print("PyMailMessage.addRecipient", recipient, file=sys.stderr)
        self.recipients.append(recipient)
    def addCcRecipient( self, ccrecipient ):
        if dbg:
            print("PyMailMessage.addCcRecipient", ccrecipient, file=sys.stderr)
        self.ccrecipients.append(ccrecipient)
    def addBccRecipient( self, bccrecipient ):
        if dbg:
            print("PyMailMessage.addBccRecipient", bccrecipient, file=sys.stderr)
        self.bccrecipients.append(bccrecipient)
    def getRecipients( self ):
        if dbg:
            print("PyMailMessage.getRecipients", self.recipients, file=sys.stderr)
        return tuple(self.recipients)
    def getCcRecipients( self ):
        if dbg:
            print("PyMailMessage.getCcRecipients", self.ccrecipients, file=sys.stderr)
        return tuple(self.ccrecipients)
    def getBccRecipients( self ):
        if dbg:
            print("PyMailMessage.getBccRecipients", self.bccrecipients, file=sys.stderr)
        return tuple(self.bccrecipients)
    def addAttachment( self, aMailAttachment ):
        if dbg:
            print("PyMailMessage.addAttachment", file=sys.stderr)
        self.aMailAttachments.append(aMailAttachment)
    def getAttachments( self ):
        if dbg:
            print("PyMailMessage.getAttachments", file=sys.stderr)
        return tuple(self.aMailAttachments)


# pythonloader looks for a static g_ImplementationHelper variable
g_ImplementationHelper = unohelper.ImplementationHelper()
g_ImplementationHelper.addImplementation( \
        PyMailServiceProvider, "org.openoffice.pyuno.MailServiceProvider",
                ("com.sun.star.mail.MailServiceProvider",),)
g_ImplementationHelper.addImplementation( \
        PyMailMessage, "org.openoffice.pyuno.MailMessage",
                ("com.sun.star.mail.MailMessage",),)
