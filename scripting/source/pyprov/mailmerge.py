#!/bin/python

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

import sys, smtplib, imaplib, poplib

dbg = False

class PyMailSMTPService(unohelper.Base, XSmtpService):
	def __init__( self, ctx ):
		self.ctx = ctx
		self.listeners = []
		self.supportedtypes = ('Insecure', 'Ssl')
		self.server = None
		self.connectioncontext = None
		self.notify = EventObject()
		if dbg:
			print >> sys.stderr, "PyMailSMPTService init"
	def addConnectionListener(self, xListener):
		if dbg:
			print >> sys.stderr, "PyMailSMPTService addConnectionListener"
		self.listeners.append(xListener)
	def removeConnectionListener(self, xListener):
		if dbg:
			print >> sys.stderr, "PyMailSMPTService removeConnectionListener"
		self.listeners.remove(xListener)
	def getSupportedConnectionTypes(self):
		if dbg:
			print >> sys.stderr, "PyMailSMPTService getSupportedConnectionTypes"
		return self.supportedtypes
	def connect(self, xConnectionContext, xAuthenticator):
		self.connectioncontext = xConnectionContext
		if dbg:
			print >> sys.stderr, "PyMailSMPTService connect"
		server = xConnectionContext.getValueByName("ServerName")
		if dbg:
			print >> sys.stderr, server
		port = xConnectionContext.getValueByName("Port")
		if dbg:
			print >> sys.stderr, port
		self.server = smtplib.SMTP(server, port)
		if dbg:
			self.server.set_debuglevel(1)
		connectiontype = xConnectionContext.getValueByName("ConnectionType")
		if dbg:
			print >> sys.stderr, connectiontype
		if connectiontype == 'Ssl':
			self.server.starttls()

		user = xAuthenticator.getUserName()
		password = xAuthenticator.getPassword()
		if user != '':
			if dbg:
				print >> sys.stderr, 'Logging in, username of', user
			self.server.login(user, password)

		for listener in self.listeners:
			listener.connected(self.notify)
	def disconnect(self):
		if dbg:
			print >> sys.stderr, "PyMailSMPTService disconnect"
		if self.server:
			self.server.quit()
			self.server = None
		for listener in self.listeners:
			listener.disconnected(self.notify)
	def isConnected(self):
		if dbg:
			print >> sys.stderr, "PyMailSMPTService isConnected"
		return self.server != None
	def getCurrentConnectionContext(self):
		if dbg:
			print >> sys.stderr, "PyMailSMPTService getCurrentConnectionContext"
		return self.connectioncontext
	def sendMailMessage(self, xMailMessage):
		COMMASPACE = ', '

		if dbg:
			print >> sys.stderr, "PyMailSMPTService sendMailMessage"
		recipients = xMailMessage.getRecipients()
		sendermail = xMailMessage.SenderAddress
                sendername = xMailMessage.SenderName
		subject = xMailMessage.Subject
		ccrecipients = xMailMessage.getCcRecipients()
		bccrecipients = xMailMessage.getBccRecipients()
		if dbg:
			print >> sys.stderr, "PyMailSMPTService subject", subject
			print >> sys.stderr, "PyMailSMPTService from", sendername.encode('utf-8')
                        print >> sys.stderr, "PyMailSMTPService from", sendermail
			print >> sys.stderr, "PyMailSMPTService send to", recipients

		attachments = xMailMessage.getAttachments()

		content = xMailMessage.Body
		flavors = content.getTransferDataFlavors()
		flavor = flavors[0]
		if dbg:
			print >> sys.stderr, "PyMailSMPTService mimetype is", flavor.MimeType
		textbody = content.getTransferData(flavor)

		textmsg = Message()
		mimeEncoding = re.sub("charset=.*", "charset=UTF-8", flavor.MimeType)
		textmsg['Content-Type'] = mimeEncoding
	        textmsg['MIME-Version'] = '1.0'
		textmsg.set_payload(textbody.encode('utf-8'))

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
		msg['X-Mailer'] = "OpenOffice.org 2.0 via Caolan's mailmerge component"

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
			msgattachment.add_header('Content-Disposition', 'attachment', \
				filename=attachment.ReadableName)
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
		truerecipients = uniquer.keys()

		if dbg:
			print >> sys.stderr, "PyMailSMPTService recipients are", truerecipients

		self.server.sendmail(sendermail, truerecipients, msg.as_string())

class PyMailIMAPService(unohelper.Base, XMailService):
	def __init__( self, ctx ):
		self.ctx = ctx
		self.listeners = []
		self.supportedtypes = ('Insecure', 'Ssl')
		self.server = None
		self.connectioncontext = None
		if dbg:
			print >> sys.stderr, "PyMailIMAPService init"
	def addConnectionListener(self, xListener):
		if dbg:
			print >> sys.stderr, "PyMailIMAPService addConnectionListener"
		self.listeners.append(xListener)
	def removeConnectionListener(self, xListener):
		if dbg:
			print >> sys.stderr, "PyMailIMAPService removeConnectionListener"
		self.listeners.remove(xListener)
	def getSupportedConnectionTypes(self):
		if dbg:
			print >> sys.stderr, "PyMailIMAPService getSupportedConnectionTypes"
		return self.supportedtypes
	def connect(self, xConnectionContext, xAuthenticator):
		if dbg:
			print >> sys.stderr, "PyMailIMAPService connect"

		self.connectioncontext = xConnectionContext
		server = xConnectionContext.getValueByName("ServerName")
		if dbg:
			print >> sys.stderr, server
		port = xConnectionContext.getValueByName("Port")
		if dbg:
			print >> sys.stderr, port
		connectiontype = xConnectionContext.getValueByName("ConnectionType")
		if dbg:
			print >> sys.stderr, connectiontype
		print >> sys.stderr, "BEFORE"
		if connectiontype == 'Ssl':
			self.server = imaplib.IMAP4_SSL(server, port)
		else:
			self.server = imaplib.IMAP4(server, port)
		print >> sys.stderr, "AFTER"
			
		user = xAuthenticator.getUserName()
		password = xAuthenticator.getPassword()
		if user != '':
			if dbg:
				print >> sys.stderr, 'Logging in, username of', user
			self.server.login(user, password)

		for listener in self.listeners:
			listener.connected(self.notify)
	def disconnect(self):
		if dbg:
			print >> sys.stderr, "PyMailIMAPService disconnect"
		if self.server:
			self.server.logout()
			self.server = None
		for listener in self.listeners:
			listener.disconnected(self.notify)
	def isConnected(self):
		if dbg:
			print >> sys.stderr, "PyMailIMAPService isConnected"
		return self.server != None
	def getCurrentConnectionContext(self):
		if dbg:
			print >> sys.stderr, "PyMailIMAPService getCurrentConnectionContext"
		return self.connectioncontext

class PyMailPOP3Service(unohelper.Base, XMailService):
	def __init__( self, ctx ):
		self.ctx = ctx
		self.listeners = []
		self.supportedtypes = ('Insecure', 'Ssl')
		self.server = None
		self.connectioncontext = None
		if dbg:
			print >> sys.stderr, "PyMailPOP3Service init"
	def addConnectionListener(self, xListener):
		if dbg:
			print >> sys.stderr, "PyMailPOP3Service addConnectionListener"
		self.listeners.append(xListener)
	def removeConnectionListener(self, xListener):
		if dbg:
			print >> sys.stderr, "PyMailPOP3Service removeConnectionListener"
		self.listeners.remove(xListener)
	def getSupportedConnectionTypes(self):
		if dbg:
			print >> sys.stderr, "PyMailPOP3Service getSupportedConnectionTypes"
		return self.supportedtypes
	def connect(self, xConnectionContext, xAuthenticator):
		if dbg:
			print >> sys.stderr, "PyMailPOP3Service connect"

		self.connectioncontext = xConnectionContext
		server = xConnectionContext.getValueByName("ServerName")
		if dbg:
			print >> sys.stderr, server
		port = xConnectionContext.getValueByName("Port")
		if dbg:
			print >> sys.stderr, port
		connectiontype = xConnectionContext.getValueByName("ConnectionType")
		if dbg:
			print >> sys.stderr, connectiontype
		print >> sys.stderr, "BEFORE"
		if connectiontype == 'Ssl':
			self.server = poplib.POP3_SSL(server, port)
		else:
			self.server = poplib.POP3(server, port)
		print >> sys.stderr, "AFTER"
			
		user = xAuthenticator.getUserName()
		password = xAuthenticator.getPassword()
		if dbg:
			print >> sys.stderr, 'Logging in, username of', user
		self.server.user(user)
		self.server.pass_(user, password)

		for listener in self.listeners:
			listener.connected(self.notify)
	def disconnect(self):
		if dbg:
			print >> sys.stderr, "PyMailPOP3Service disconnect"
		if self.server:
			self.server.quit()
			self.server = None
		for listener in self.listeners:
			listener.disconnected(self.notify)
	def isConnected(self):
		if dbg:
			print >> sys.stderr, "PyMailPOP3Service isConnected"
		return self.server != None
	def getCurrentConnectionContext(self):
		if dbg:
			print >> sys.stderr, "PyMailPOP3Service getCurrentConnectionContext"
		return self.connectioncontext

class PyMailServiceProvider(unohelper.Base, XMailServiceProvider):
	def __init__( self, ctx ):
		if dbg:
			print >> sys.stderr, "PyMailServiceProvider init"
		self.ctx = ctx
	def create(self, aType):
		if dbg:
			print >> sys.stderr, "PyMailServiceProvider create with", aType
		if aType == SMTP:
			return PyMailSMTPService(self.ctx);
		elif aType == POP3:
			return PyMailPOP3Service(self.ctx);
		elif aType == IMAP:
			return PyMailIMAPService(self.ctx);
		else:
			print >> sys.stderr, "PyMailServiceProvider, unknown TYPE", aType

# pythonloader looks for a static g_ImplementationHelper variable
g_ImplementationHelper = unohelper.ImplementationHelper()
g_ImplementationHelper.addImplementation( \
	PyMailServiceProvider, "org.openoffice.pyuno.MailServiceProvider",
		("com.sun.star.mail.MailServiceProvider",),)

