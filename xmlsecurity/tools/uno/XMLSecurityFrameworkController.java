/*************************************************************************
 *
 *  $RCSfile: XMLSecurityFrameworkController.java,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: mt $ $Date: 2004-07-12 13:15:24 $
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
 *  This library is distributed in the hope that it will be useful,
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
 *  except in compliance with the License. You may obtain a copy of the
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

package com.sun.star.xml.security.uno;

import java.util.Stack;
import java.util.Vector;

/* uno classes */
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XInitialization;
import com.sun.star.uno.XComponentContext;
import com.sun.star.xml.sax.XDocumentHandler;
import com.sun.star.xml.sax.XAttributeList;
import com.sun.star.xml.sax.SAXException;

import com.sun.star.xml.crypto.*;
import com.sun.star.xml.crypto.sax.*;
import com.sun.star.xml.wrapper.*;

/*
 * the XMLSecurityFrameworkController class is used to controll the xml security framework.
 */
public class XMLSecurityFrameworkController
    implements XDocumentHandler, XSignatureCreationResultListener, XSignatureVerifyResultListener,
           XEncryptionResultListener, XDecryptionResultListener, XSAXEventKeeperStatusChangeListener
{
    /*
     * UNO framework component
     */
    private XMultiComponentFactory  m_xRemoteServiceManager;
    private XComponentContext       m_xRemoteContext;

    /*
     * xml security related UNO components
     */
    private XSecuritySAXEventKeeper m_xSAXEventKeeper;
    private XXMLDocumentWrapper     m_xXMLDocumentWrapper;
    private XDocumentHandler        m_xOutputHandler;
    private XXMLSecurityContext     m_xXMLSecurityContext;
    private XXMLSignature           m_xXMLSignature;
    private XXMLEncryption          m_xXMLEncryption;

        /*
         * used to reserve the current SAX ancestor path
         */
    private Stack  m_currentPath;

    /*
     * maintains all SignatureEntities.
     */
    private Vector m_signatureList;

    /*
     * maintains all EncryptionEntities.
     */
    private Vector m_encryptionList;

    /*
     * maintains all unsolved reference Ids.
     * These ids are strings which is the value of the id attribute
     * of the referenced element.
     */
    private Vector m_vUnsolvedReferenceIds;

    /*
     * maintains all unsolved reference keeper ids.
     * The keeper id is used to uniquely identify a bufferred element
     * by the SAXEventKeeper.
     */
    private Vector m_vUnsolvedReferencedKeeperIds;

    /*
     * maintains the left time that each unsolved reference can be
     * claimed.
     */
    private Vector m_vUnsolvedReferenceRefNum;

    /*
     * whether exporting or importing
     */
    private boolean m_bIsExporting;

    /*
     * whether java or c
     */
    private boolean m_bIsJavaBased;

    /*
     * whether the SAXEventKeeper is blocking
     */
    private boolean m_bIsBlocking;

    /*
     * whether it is collecting a bufferred element
     */
    private boolean m_bIsInsideCollectedElement;

    /*
     * whether a SAXEventKeeper is in the SAX chain
     */
    private boolean m_bSAXEventKeeperIncluded;

    /*
     * the ParsingThread used to parse the document
     */
    private ParsingThread m_parsingThread;

    /*
     * the next document handler that will receives SAX events
     * from the parsing thread.
     * if the SAXEventKeeper is on the SAX chain, then this
     * variable will be the SAXEventKeeper, otherwise, this
     * variable will be the xOutputHandler.
     */
    private XDocumentHandler m_xExportHandler;

    /*
     * the TestTool used to feedback information
     */
    private TestTool m_testTool;

    /*
     * for encryption target
     */
    private boolean m_bIsEncryptionTarget;
    private EncryptionEntity m_EncryptionForTarget;

    XMLSecurityFrameworkController(
        TestTool testTool,
        boolean bIsExporting,
        boolean bIsJavaBased,
        XDocumentHandler xOutputHandler,
        ParsingThread parsingThread,
        XXMLSecurityContext xXMLSecurityContext,
        XXMLSignature xXMLSignature,
        XXMLEncryption xXMLEncryption,
        XMultiComponentFactory xRemoteServiceManager,
        XComponentContext xRemoteContext)
    {
        m_bIsExporting = bIsExporting;
        m_bIsJavaBased = bIsJavaBased;

        m_xOutputHandler = xOutputHandler;
        m_xXMLSecurityContext = xXMLSecurityContext;
        m_xXMLSignature = xXMLSignature;
        m_xXMLEncryption = xXMLEncryption;
        m_xRemoteServiceManager = xRemoteServiceManager;
        m_xRemoteContext = xRemoteContext;

        m_testTool = testTool;
        m_parsingThread = parsingThread;

        m_signatureList = new Vector();
        m_encryptionList = new Vector();

        m_vUnsolvedReferenceIds = new Vector();
        m_vUnsolvedReferencedKeeperIds = new Vector();
        m_vUnsolvedReferenceRefNum = new Vector();

        m_xXMLDocumentWrapper = null;
        m_xSAXEventKeeper = null;

        m_bSAXEventKeeperIncluded = false;
        m_bIsBlocking = false;
        m_bIsInsideCollectedElement = false;

        m_bIsEncryptionTarget = false;
        m_EncryptionForTarget = null;

        changeOutput();

        m_currentPath = new Stack();

        foundSecurityRelated();
    }

/**************************************************************************************
 * private methods
 **************************************************************************************/

        /*
         * changes the output document handler.
         */
        private void changeOutput()
        {
        if (m_bIsExporting)
        {
            m_parsingThread.setHandler(this);

            /*
             * If the SAXEventKeeper is in the SAX chain, then redirects output
             * to the SAXEventKeeper, otherwise, to the m_xOutputHandler
             */
            if (m_bSAXEventKeeperIncluded)
            {
                m_xExportHandler = (XDocumentHandler)UnoRuntime.queryInterface(
                            XDocumentHandler.class, m_xSAXEventKeeper);
                m_xSAXEventKeeper.setNextHandler(m_xOutputHandler);

                m_testTool.updatesSAXChainInformation("XMLExporter -> SAXEventKeeper -> SAXWriter");
            }
            else
            {
                m_xExportHandler = m_xOutputHandler;
                m_testTool.updatesSAXChainInformation("XMLExporter -> SAXWriter");
            }
        }
        else
        {
            if (m_bSAXEventKeeperIncluded)
            {
                m_parsingThread.setHandler(
                    (XDocumentHandler)UnoRuntime.queryInterface(XDocumentHandler.class, m_xSAXEventKeeper));
                m_xSAXEventKeeper.setNextHandler(this);
                m_testTool.updatesSAXChainInformation("SAXParser -> SAXEventKeeper -> XMLImporter");
            }
            else
            {
                m_parsingThread.setHandler(this);
                m_testTool.updatesSAXChainInformation("SAXParser -> XMLImporter");
            }
            m_xExportHandler = m_xOutputHandler;
        }
    }

        /*
         * handles the situation when a security related element is found.
         * if the SAXEventKeeper is not initialized, then creates a
         * SAXEventKeeper.
         * the return value represents whether the SAXEventKeeper is newly
         * created.
         */
    private boolean foundSecurityRelated()
    {
        if (m_xSAXEventKeeper == null)
        {
            m_testTool.showMessage("Message from : "+
                        (m_bIsExporting?"XMLExporter":"XMLImporter")+
                        "\n\nA security related content found, a SAXEventKeeper is created.\n ");

            m_bIsBlocking = false;
            m_bIsInsideCollectedElement = false;

            try
            {
                /*
                 * creates an XMLDocumentWrapper component.
                 */
                Object xmlDocumentObj = null;

                if (m_bIsJavaBased)
                {
                    xmlDocumentObj = m_xRemoteServiceManager.createInstanceWithContext(
                        TestTool.XMLDOCUMENTWRAPPER_COMPONENT_JAVA, m_xRemoteContext);
                }
                else
                {
                    xmlDocumentObj = m_xRemoteServiceManager.createInstanceWithContext(
                        TestTool.XMLDOCUMENTWRAPPER_COMPONENT_C, m_xRemoteContext);
                }

                m_xXMLDocumentWrapper = (XXMLDocumentWrapper)UnoRuntime.queryInterface(
                    XXMLDocumentWrapper.class, xmlDocumentObj);

                /*
                 * creates a SAXEventKeeper component.
                 */
                Object saxEventKeeperObj = m_xRemoteServiceManager.createInstanceWithContext(
                    TestTool.SAXEVENTKEEPER_COMPONENT, m_xRemoteContext);

                m_xSAXEventKeeper =
                    (XSecuritySAXEventKeeper)UnoRuntime.queryInterface(
                        XSecuritySAXEventKeeper.class, saxEventKeeperObj);

                            /*
                             * initializes the SAXEventKeeper component with the XMLDocumentWrapper component.
                             */
                XInitialization xInitialization =
                    (XInitialization)UnoRuntime.queryInterface(
                        XInitialization.class, m_xSAXEventKeeper);
                Object args[]=new Object[1];
                args[0] = m_xXMLDocumentWrapper;
                xInitialization.initialize(args);
            }
            catch( com.sun.star.uno.Exception e)
            {
                e.printStackTrace();
            }

            /*
             * configures the SAXEventKeeper's status change listener.
             */
            XSAXEventKeeperStatusChangeBroadcaster xSaxEventKeeperStatusChangeBroadcaster =
                (XSAXEventKeeperStatusChangeBroadcaster)UnoRuntime.queryInterface(
                    XSAXEventKeeperStatusChangeBroadcaster.class, m_xSAXEventKeeper);
            xSaxEventKeeperStatusChangeBroadcaster.addXSAXEventKeeperStatusChangeListener(this);
        }

        boolean rc = !m_bSAXEventKeeperIncluded;

        /*
         * changes the export document handler.
         */
        m_bSAXEventKeeperIncluded=true;
        changeOutput();

        return rc;
    }

    /*
     * finds key element or referenced element for a signature.
     */
    private void findKeyOrReference(SecurityEntity signatureEntity, String uriStr, boolean isFindingKey)
    {
        int i=0;

        while (i<m_vUnsolvedReferenceIds.size())
        {
            String id = (String)m_vUnsolvedReferenceIds.elementAt(i);

            if (id.equals(uriStr))
            {
                int refNum = ((Integer)m_vUnsolvedReferenceRefNum.elementAt(i)).intValue();
                int keeperId = ((Integer)m_vUnsolvedReferencedKeeperIds.elementAt(i)).intValue();

                if (isFindingKey)
                {
                    /*
                     * clones a new ElementCollector for the key element.
                     */
                    int cloneKeeperId = m_xSAXEventKeeper.cloneElementCollector(
                        keeperId,
                        m_bIsExporting?
                        (ElementMarkPriority.PRI_BEFOREMODIFY):(ElementMarkPriority.PRI_AFTERMODIFY));

                    /*
                     * notifies the key keeper id.
                     */
                    signatureEntity.setKeyId(cloneKeeperId);

                    /*
                     * sets the security id for the key.
                     */
                    m_xSAXEventKeeper.setSecurityId(cloneKeeperId, signatureEntity.getSecurityId());

                    /*
                     * sets the resolve listener.
                     */
                    XReferenceResolvedBroadcaster xReferenceResolvedBroadcaster =
                        (XReferenceResolvedBroadcaster)UnoRuntime.queryInterface(
                            XReferenceResolvedBroadcaster.class, m_xSAXEventKeeper);
                    xReferenceResolvedBroadcaster.addReferenceResolvedListener(
                        cloneKeeperId,
                        signatureEntity.getReferenceListener());
                }
                else
                {
                    /*
                     * clones a new ElementCollector for the referenced element.
                     */
                    int cloneKeeperId = m_xSAXEventKeeper.cloneElementCollector(
                        keeperId,
                        m_bIsExporting?
                        (ElementMarkPriority.PRI_AFTERMODIFY):(ElementMarkPriority.PRI_BEFOREMODIFY));

                    /*
                     * sets the security id.
                     */
                    m_xSAXEventKeeper.setSecurityId(cloneKeeperId, signatureEntity.getSecurityId());

                    /*
                     * sets the resolve listener.
                     */
                    XReferenceResolvedBroadcaster xReferenceResolvedBroadcaster =
                        (XReferenceResolvedBroadcaster)UnoRuntime.queryInterface(
                            XReferenceResolvedBroadcaster.class, m_xSAXEventKeeper);
                    xReferenceResolvedBroadcaster.addReferenceResolvedListener(cloneKeeperId,
                        signatureEntity.getReferenceListener());

                    try{
                        XReferenceCollector xReferenceCollector =
                            (XReferenceCollector)UnoRuntime.queryInterface(
                                XReferenceCollector.class, signatureEntity.getReferenceListener());
                        xReferenceCollector.setReferenceId(cloneKeeperId);
                    }
                    catch( com.sun.star.uno.Exception e)
                    {
                        e.printStackTrace();
                    }
                }

                /*
                 * if this unsolved reference reaches its max reference number, remove this reference
                 * from all vectors.
                 */
                refNum--;
                if (refNum == 0)
                {
                    m_xSAXEventKeeper.removeElementCollector(keeperId);
                    m_vUnsolvedReferenceIds.remove(i);
                    m_vUnsolvedReferencedKeeperIds.remove(i);
                    m_vUnsolvedReferenceRefNum.remove(i);
                }
                else
                {
                    m_vUnsolvedReferenceRefNum.setElementAt(new Integer(refNum),(i));
                    ++i;
                }

                /*
                 * If it is find a key, then no further search is needed, one
                 * signature has one key at most.
                 */
                if (isFindingKey)
                {
                    break;
                }
            }
            else
            {
                ++i;
            }
        }
    }

    /*
     * checks whether a startElement event represents any security related information.
     * return true if this event can't be forwarded into the SAX chain.
     */
    private boolean checkSecurityElement(String localName, com.sun.star.xml.sax.XAttributeList xattribs)
    {
        boolean rc = false;

        if (localName.equals("Signature"))
        /*
         * this element is a Signature element.
         */
        {
            SignatureEntity signatureEntity = new SignatureEntity(
                m_xSAXEventKeeper,
                m_bIsExporting,
                this,
                m_xXMLSecurityContext,
                m_xXMLSignature,
                m_xXMLEncryption,
                m_xRemoteServiceManager,
                m_xRemoteContext);

            m_signatureList.add(signatureEntity);
            m_currentPath.push(signatureEntity);
        }
        else if(localName.equals("Reference"))
        {
            if (!m_currentPath.empty())
            {
                Object signedInfo = m_currentPath.pop();

                if (!m_currentPath.empty())
                {
                    Object objSignature = m_currentPath.peek();

                    if ((objSignature instanceof SignatureEntity) && signedInfo.toString().equals("SignedInfo"))
                    /*
                     * this element is a Reference element in a signature.
                     */
                    {
                        String uriStr = xattribs.getValueByName("URI");

                        if (uriStr.charAt(0) == '#')
                        {
                            uriStr = uriStr.substring(1);
                            SignatureEntity signatureEntity = (SignatureEntity)objSignature;

                            if (uriStr != null && uriStr.length()>0)
                            {
                                signatureEntity.addReferenceId(uriStr);
                                findKeyOrReference(signatureEntity, uriStr, false);
                            }
                        }
                    }
                }
                m_currentPath.push(signedInfo);
            }
            m_currentPath.push(localName);
        }
        else if(localName.equals("KeyValue") ||
                localName.equals("KeyName") ||
                localName.equals("X509Data") ||
                localName.equals("EncryptedKey"))
        {
            if (!m_currentPath.empty())
            {
                Object keyInfo = m_currentPath.pop();

                if (!m_currentPath.empty())
                {
                    Object objSorE = m_currentPath.peek();

                    if ((objSorE instanceof SignatureEntity) && keyInfo.toString().equals("KeyInfo"))
                    /*
                     * this element is the key element of a signature.
                     */
                    {
                        SignatureEntity signatureEntity = (SignatureEntity)objSorE;
                        signatureEntity.setKeyId(0);
                    }
                    else if ((objSorE instanceof EncryptionEntity) && keyInfo.toString().equals("KeyInfo"))
                    /*
                     * this element is the key element of an encryption.
                     */
                    {
                        EncryptionEntity theEncryption = (EncryptionEntity)objSorE;
                        theEncryption.setKeyId(0);
                    }
                }
                m_currentPath.push(keyInfo);
            }

            m_currentPath.push(localName);
        }
        else if(localName.equals("RetrievalMethod"))
        {
            if (!m_currentPath.empty())
            {
                Object keyInfo = m_currentPath.pop();

                if (!m_currentPath.empty())
                {
                    Object objSorE = m_currentPath.peek();

                    if ((objSorE instanceof SignatureEntity) && keyInfo.toString().equals("KeyInfo"))
                    /*
                     * this element is the RetrievalMethod element in a signature,
                     * which will include the key uri of this signature.
                     */
                    {
                        String uriStr = xattribs.getValueByName("URI");
                        SignatureEntity signatureEntity = (SignatureEntity)objSorE;

                        if (uriStr != null && uriStr.length()>0)
                        {
                            signatureEntity.setKeyURI(uriStr);
                            findKeyOrReference(signatureEntity,uriStr, true);
                        }
                    }
                    else if ((objSorE instanceof EncryptionEntity) && keyInfo.toString().equals("KeyInfo"))
                    /*
                     * this element is the RetrievalMethod element in an encryption,
                     * which will include the key uri of this encryption.
                     */
                    {
                        String uriStr = xattribs.getValueByName("URI");
                        EncryptionEntity theEncryption = (EncryptionEntity)objSorE;

                        if (uriStr != null && uriStr.length()>0)
                        {
                            theEncryption.setKeyURI(uriStr);
                            findKeyOrReference(theEncryption, uriStr, true);
                        }
                    }
                }
                m_currentPath.push(keyInfo);
            }
            m_currentPath.push(localName);
        }
        else if (localName.equals("EncryptedData")) /* || localName.equals("EncryptedKey")) */
        /*
         * this element is an Encryption element.
         */
        {
            EncryptionEntity theEncryption = new EncryptionEntity(
                m_xSAXEventKeeper,
                m_bIsExporting,
                this,
                m_xXMLSecurityContext,
                m_xXMLSignature,
                m_xXMLEncryption,
                m_xRemoteServiceManager,
                m_xRemoteContext);

            m_encryptionList.add(theEncryption);

            if (m_bIsExporting)
            {
                m_currentPath.push(theEncryption);
            }
            else
            {
                String uriStr = xattribs.getValueByName("keyURI");
                if (uriStr != null && uriStr.length()>0)
                {
                    theEncryption.setKeyURI(uriStr);
                    findKeyOrReference(theEncryption,uriStr, true);
                }
                else
                {
                    theEncryption.setKeyId(0);
                }

                rc = true;
            }
        }
        else
        /*
         * not a security related element.
         */
        {
            m_currentPath.push(localName);
        }

        return rc;
    }

    /*
     * checks whether a startElement event is referenced by any security entity.
     */
    private void checkReference(String localName, com.sun.star.xml.sax.XAttributeList xattribs, String id)
    {
        String refNumStr = xattribs.getValueByName("refNum");

        if ( m_bIsEncryptionTarget )
        {
            m_EncryptionForTarget.setReference(m_bIsExporting);
            m_bIsEncryptionTarget = false;
        }

        if (id != null && id.length()>0 )
        /*
         * only if this element has id attribute, then it can be referenced by
         * a security entity.
         */
        {
            /*
             * if this element has an "refNum" attribute, then the value will be
             * the max referencing number on this element, otherwise, set the max
             * referencing number to 999.
             */
            int refNum = 999;

            if (refNumStr != null && refNumStr.length()>0 )
            {
                refNum = new Integer(refNumStr).intValue();
            }

            int length;

            /*
             * searches the signature list to check whether any sigture has
             * reference on this element.
             */
            length = m_signatureList.size();
            for (int i=0; i<length; ++i)
            {
                SignatureEntity signatureEntity = (SignatureEntity)m_signatureList.elementAt(i);

                if (signatureEntity.setReference(id, m_bIsExporting))
                {
                    refNum--;
                }

                if (signatureEntity.setKey(id, m_bIsExporting))
                {
                    refNum--;
                }
            }

            /*
             * searches the encryption list for reference.
             */
            length = m_encryptionList.size();
            for (int i=0; i<length; ++i)
            {
                EncryptionEntity theEncryption = (EncryptionEntity)m_encryptionList.elementAt(i);

                if (theEncryption.setKey(id, m_bIsExporting))
                {
                    refNum--;
                }
            }

            /*
             * if the max referencing number is not reached, then add this element
             * into the unsolved reference list.
             */
            if (refNum>0)
            {
                int keeperId;

                if (localName.equals("EncryptedKey"))
                {
                    keeperId = m_xSAXEventKeeper.addSecurityElementCollector(
                        m_bIsExporting?
                        (ElementMarkPriority.PRI_BEFOREMODIFY):(ElementMarkPriority.PRI_AFTERMODIFY),
                        true);
                }
                else
                {
                    keeperId = m_xSAXEventKeeper.addSecurityElementCollector(
                        m_bIsExporting?
                        (ElementMarkPriority.PRI_AFTERMODIFY):(ElementMarkPriority.PRI_BEFOREMODIFY),
                        false);
                }

                m_vUnsolvedReferenceIds.add(id);
                m_vUnsolvedReferencedKeeperIds.add(new Integer(keeperId));
                m_vUnsolvedReferenceRefNum.add(new Integer(refNum));
            }
        }
    }

    /*
     * configures the output handler.
     */
    private void setOutputHandler(XDocumentHandler handler)
    {
        m_xOutputHandler = handler;
        changeOutput();
    }


/**************************************************************************************
 * protected methods
 **************************************************************************************/

        /*
         * methods used to transfer unsolved reference information.
         */
    protected Vector getUnsolvedReferenceIds()
    {
        return m_vUnsolvedReferenceIds;
    }

    protected Vector getUnsolvedReferenceKeeperIds()
    {
        return m_vUnsolvedReferencedKeeperIds;
    }

    protected Vector getUnsolvedReferenceRefNum()
    {
        return m_vUnsolvedReferenceRefNum;
    }

    protected String getBufferNodeTreeInformation()
    {
        if (m_xSAXEventKeeper != null)
        {
            return m_xSAXEventKeeper.printBufferNodeTree();
        }
        else
        {
            return null;
        }
    }

    protected void getDocument(XDocumentHandler handler)
    {
        if (m_xXMLDocumentWrapper != null)
        {
            try
            {
                m_xXMLDocumentWrapper.getTree(handler);
            }
            catch(SAXException e)
            {
                e.printStackTrace();
            }
        }
    }

    protected void endMission()
    {
        while (m_signatureList.size()>0 || m_encryptionList.size()>0)
        {
            if (m_signatureList.size()>0)
            {
                SignatureEntity signatureEntity = (SignatureEntity)m_signatureList.elementAt(0);
                m_signatureList.remove(0);
                signatureEntity.endMission();
            }
            else if (m_encryptionList.size()>0)
            {
                EncryptionEntity theEncryption = (EncryptionEntity)m_encryptionList.elementAt(0);
                m_encryptionList.remove(0);
                theEncryption.endMission();
            }
        }

        while (m_vUnsolvedReferenceIds.size()>0)
        {
            int keeperId = ((Integer)m_vUnsolvedReferencedKeeperIds.elementAt(0)).intValue();
            m_xSAXEventKeeper.removeElementCollector(keeperId);
            m_vUnsolvedReferenceIds.remove(0);
            m_vUnsolvedReferencedKeeperIds.remove(0);
            m_vUnsolvedReferenceRefNum.remove(0);
        }

        m_xSAXEventKeeper.setNextHandler(null);

        XSAXEventKeeperStatusChangeBroadcaster xSaxEventKeeperStatusChangeBroadcaster =
            (XSAXEventKeeperStatusChangeBroadcaster)UnoRuntime.queryInterface(
                XSAXEventKeeperStatusChangeBroadcaster.class, m_xSAXEventKeeper);
        xSaxEventKeeperStatusChangeBroadcaster.addXSAXEventKeeperStatusChangeListener(null);

        m_xSAXEventKeeper = null;
        m_xXMLDocumentWrapper = null;
        m_xOutputHandler = null;
        m_xXMLSecurityContext = null;
        m_xXMLSignature = null;
        m_xXMLEncryption = null;

        m_xExportHandler = null;
        m_parsingThread.setHandler(null);
    }

/**************************************************************************************
 * public methods
 **************************************************************************************/

    /*
     * XDocumentHandler
     */
        public void startDocument()
    {
        try{
            m_xExportHandler.startDocument();
        }
        catch( com.sun.star.xml.sax.SAXException e)
        {
            e.printStackTrace();
        }

    }

        public void endDocument()
    {
        try{
            m_xExportHandler.endDocument();
        }
        catch( com.sun.star.xml.sax.SAXException e)
        {
            e.printStackTrace();
        }
    }

    public void startElement (String str, com.sun.star.xml.sax.XAttributeList xattribs)
    {
        try{
            String idAttr = xattribs.getValueByName("id");
            if (idAttr == null)
            {
                idAttr = xattribs.getValueByName("Id");
            }

            boolean hasIdAttr = (idAttr != null && idAttr.length()>0 );
            boolean needResend = false;

            if (hasIdAttr ||
                (str.equals("Signature")||str.equals("EncryptedData")))/* || str.equals("EncryptedKey"))) */
            {
                if (foundSecurityRelated() && !m_bIsExporting)
                {
                    needResend = true;
                }
            }

            boolean suppressToNext = checkSecurityElement(str, xattribs);

            checkReference(str, xattribs, idAttr);

            if (needResend)
            {
                m_xSAXEventKeeper.setNextHandler(null);

                XDocumentHandler saxEventKeeperHandler =
                    (XDocumentHandler)UnoRuntime.queryInterface(
                        XDocumentHandler.class, m_xSAXEventKeeper);
                saxEventKeeperHandler.startElement(str, xattribs);
                m_xSAXEventKeeper.setNextHandler((XDocumentHandler)this);
            }

            if (!suppressToNext)
            {
                m_xExportHandler.startElement(str, xattribs);
            }
        }
        catch( com.sun.star.xml.sax.SAXException e)
        {
            e.printStackTrace();
        }
    }

    public void endElement(String str)
    {
        if (!m_currentPath.empty())
        {
                Object obj = m_currentPath.pop();

                if (obj.toString().equals("SignedInfo"))
                {
                if (!m_currentPath.empty())
                {
                        Object objSignature = m_currentPath.peek();
                        if (objSignature != null && objSignature instanceof SignatureEntity)
                        {
                            ((SignatureEntity)objSignature).setReferenceNumber();
                        }
                    }
                }
                else if (obj instanceof EncryptionEntity)
                {
                       m_bIsEncryptionTarget = true;
                m_EncryptionForTarget = (EncryptionEntity)obj;

                }
            }

        try{
            m_xExportHandler.endElement(str);
        }
        catch( com.sun.star.xml.sax.SAXException e)
        {
            e.printStackTrace();
        }
    }

    public void characters(String str)
    {
        try{
                m_xExportHandler.characters(str);
        }
        catch( com.sun.star.xml.sax.SAXException e)
        {
            e.printStackTrace();
        }
    }

    public void ignorableWhitespace(String str)
    {
    }

    public void processingInstruction(String aTarget, String aData)
    {
        try{
            m_xExportHandler.processingInstruction(aTarget, aData);
        }
        catch( com.sun.star.xml.sax.SAXException e)
        {
            e.printStackTrace();
        }
    }

    public void setDocumentLocator (com.sun.star.xml.sax.XLocator xLocator )
        throws com.sun.star.xml.sax.SAXException
    {
    }


    /*
     * XSignatureCreationResultListener
     */
    public void signatureCreated(int securityId, SignatureCreationResult creationResult)
    {
        String message = new String();
        message += "A Signature is created:";
        message += "\nSecurity Id = "+securityId;
        message += "\nCreation result = "+((creationResult==SignatureCreationResult.CREATIONSUCCEED)?"Succeed":"Fail");

        m_testTool.showMessage("Message from : SignatureCreator\n\n"+message+"\n ");
    }

    /*
     * XSignatureVerifyResultListener
     */
    public void signatureVerified(int securityId, SignatureVerifyResult verifyResult)
    {
        String message = new String();
        message += "A Signature is verified:";
        message += "\nSecurity Id = "+securityId;
        message += "\nVerify result = "+((verifyResult==SignatureVerifyResult.VERIFYSUCCEED)?"Succeed":"Fail");

        m_testTool.showMessage("Message from : SignatureVerifier\n\n"+message+"\n ");
    }

    /*
     * XEncryptionResultListener
     */
    public void encrypted(int securityId, EncryptionResult encryptionResult)
    {
        String message = new String();
        message += "An EncryptedData is encrypted:";
        message += "\nSecurity Id = "+securityId;
        message += "\nEncrypt result = "+((encryptionResult==EncryptionResult.ENCRYPTIONSUCCEED)?"Succeed":"Fail");

        m_testTool.showMessage("Message from : Encryptor\n\n"+message+"\n ");
    }

    /*
     * XDecryptionResultListener methods
     */
    public void decrypted(int securityId, DecryptionResult decryptionResult)
    {
        String message = new String();
        message += "An EncryptedData is decrypted:";
        message += "\nSecurity Id = "+securityId;
        message += "\nDecrypt result = "+((decryptionResult==DecryptionResult.DECRYPTIONSUCCEED)?"Succeed":"Fail");

        m_testTool.showMessage("Message from : Decryptor\n\n"+message+"\n ");
    }

    /*
     * XSAXEventKeeperStatusChangeListener methods
     */
    public void blockingStatusChanged(boolean isBlocking)
    {
        m_testTool.showMessage("Message from : SAXEventKeeper\n\n"+
                    (isBlocking?"The SAX event stream is blocked.":"The SAX event stream is unblocked.")+
                    "\n ");

        this.m_bIsBlocking = isBlocking;
    }

    public void collectionStatusChanged(boolean isInsideCollectedElement)
    {
        m_testTool.showMessage("Message from : SAXEventKeeper\n\n"+
                    (isInsideCollectedElement?"Begin to buffer data ...":"End of data bufferring.")+
                    "\n ");

        /*
        this.m_bIsInsideCollectedElement = isInsideCollectedElement;

        if ( !m_bIsInsideCollectedElement && !m_bIsBlocking)
        {
            m_bSAXEventKeeperIncluded = false;
        }
        else
        {
            m_bSAXEventKeeperIncluded = true;
        }
        changeOutput();
        */
    }

    public void bufferStatusChanged(boolean isBufferEmpty)
    {
        m_testTool.showMessage("Message from : SAXEventKeeper\n\n"+
                    (isBufferEmpty?"All bufferred data are released, the SAXEventKeeper is destroyed.":"buffer data appears.")+
                    "\n ");
        /*
        if (isBufferEmpty)
        {
            m_xXMLDocumentWrapper = null;
            m_xSAXEventKeeper = null;
            m_bSAXEventKeeperIncluded = false;
            changeOutput();
        }
        */
    }
}

