/*************************************************************************
 *
 *  $RCSfile: cfglocal.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:13:43 $
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
#define _PRIVATE_TEST_

#include <memory.h>
#include <string.h>
#include <stdio.h>

#ifndef _VOS_SOCKET_HXX_
#include <vos/socket.hxx>
#endif

#ifndef _CONFIGMGR_MISC_ATTRIBUTES_HXX_
#include "attributes.hxx"
#endif

#ifndef _CONFIGMGR_SESSION_REMOTESESSION_HXX_
#include "remotesession.hxx"
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif

#ifndef _CPPUHELPER_SERVICEFACTORY_HXX_
#include <cppuhelper/servicefactory.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _VOS_CONDITN_HXX_
#include <vos/conditn.hxx>
#endif

#include <osl/time.h>

#include "localsession.hxx"
#include "confname.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::xml;
using namespace ::vos;
using namespace ::cppu;
using namespace ::configmgr;
using namespace rtl;

// #define USE_LAYOUT_NODE

//=============================================================================
//= a dirty littly class for printing ascii characters
//=============================================================================
class OAsciiOutput
{
protected:
    sal_Char*   m_pCharacters;

public:
    OAsciiOutput(const ::rtl::OUString& _rUnicodeChars);
    ~OAsciiOutput() { delete m_pCharacters; }

    const sal_Char* getCharacters() const { return m_pCharacters; }
};

//-----------------------------------------------------------------------------
OAsciiOutput::OAsciiOutput(const ::rtl::OUString& _rUnicodeChars)
{
    sal_Int32 nLen = _rUnicodeChars.getLength();
    m_pCharacters = new sal_Char[nLen + 1];
    sal_Char* pFillPtr = m_pCharacters;
    const sal_Unicode* pSourcePtr = _rUnicodeChars.getStr();
#ifdef DBG_UTIL
    sal_Bool bAsserted = sal_False;
#endif
    for (sal_Int32 i=0; i<nLen; ++i, ++pFillPtr, ++pSourcePtr)
    {
        OSL_ENSHURE(bAsserted || !(bAsserted = (*pSourcePtr >= 0x80)),
            "OAsciiOutput::OAsciiOutput : non-ascii character found !");
        *pFillPtr = *reinterpret_cast<const sal_Char*>(pSourcePtr);
    }
    *pFillPtr = 0;
}

#define ASCII_STRING(rtlOUString)   OAsciiOutput(rtlOUString).getCharacters()
#define UNI_STRING(salCharPtr)      ::rtl::OUString::createFromAscii(salCharPtr)

//=============================================================================
//= OOpenNodeCallback
//=============================================================================
typedef ::cppu::WeakImplHelper1< sax::XDocumentHandler > OOpenNodeCallback_Base;
class OOpenNodeCallback : public IOpenObjectCallback, public OOpenNodeCallback_Base
{
protected:
    OUString                m_sNodeId;
    ::vos::OCondition&      m_rFinishCondition;

    enum ACTION { STARTELEMENT, CHARACTERS, ENDELEMENT };
    ACTION                  m_eLastAction;
    sal_Int32               m_nLevel;
    sal_Bool                m_bCloseStartTag;

protected:
    ~OOpenNodeCallback()
    {
    }

public:
    OOpenNodeCallback(::vos::OCondition& _rFinishCond) : m_rFinishCondition(_rFinishCond),  m_nLevel(0), m_eLastAction(ENDELEMENT), m_bCloseStartTag(sal_False) { }

    OUString getNodeId() const { return m_sNodeId; }

    // IOpenObjectCallback
    virtual void    gotObjectId(const OUString &aName);

    // IDataRequestCallback
    virtual     Reference< sax::XDocumentHandler > getDataReader() { return static_cast< sax::XDocumentHandler* >(this); }

    // IRequestCallback
    virtual     void acknowledged(sal_Int32 _nTransId);
    virtual     void failed(sal_Int32 _nErrorCode);
    virtual     void done(const StatusInfo& _rStatus);

    // IInterface
    virtual void SAL_CALL acquire(  ) throw (::com::sun::star::uno::RuntimeException) { OOpenNodeCallback_Base::acquire(); }
    virtual void SAL_CALL release(  ) throw (::com::sun::star::uno::RuntimeException) { OOpenNodeCallback_Base::release(); }

    // XDocumentHandler
    virtual void SAL_CALL startDocument(  ) throw(sax::SAXException, RuntimeException) { }
    virtual void SAL_CALL endDocument(  ) throw(sax::SAXException, RuntimeException) { }
    virtual void SAL_CALL startElement( const ::rtl::OUString& aName, const Reference< sax::XAttributeList >& xAttribs ) throw(sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement( const ::rtl::OUString& aName ) throw(sax::SAXException, RuntimeException);
    virtual void SAL_CALL characters( const ::rtl::OUString& aChars ) throw(sax::SAXException, RuntimeException);
    virtual void SAL_CALL ignorableWhitespace( const ::rtl::OUString& aWhitespaces ) throw(sax::SAXException, RuntimeException) { }
    virtual void SAL_CALL processingInstruction( const ::rtl::OUString& aTarget, const ::rtl::OUString& aData ) throw(sax::SAXException, RuntimeException) { }
    virtual void SAL_CALL setDocumentLocator( const Reference< sax::XLocator >& xLocator ) throw(sax::SAXException, RuntimeException) { }
};

//.............................................................................
inline void linefeed()
{
    // printf("\n");
}

//.............................................................................
void printTabs(sal_Int32 _nCount)
{
    sal_Char* pBuffer = new sal_Char[2 * _nCount + 1];
    memset(pBuffer, ' ', 2 * _nCount);
    pBuffer[2 * _nCount] = 0;
    // printf(pBuffer);
}

//-----------------------------------------------------------------------------
void SAL_CALL OOpenNodeCallback::startElement( const ::rtl::OUString& _rName, const Reference< sax::XAttributeList >& xAttribs ) throw(sax::SAXException, RuntimeException)
{
    switch (m_eLastAction)
    {
        case STARTELEMENT:
            if (m_bCloseStartTag)
            {
                // printf(">");
            }

            m_bCloseStartTag = sal_False;
            // no break
        case CHARACTERS:
        case ENDELEMENT:
            linefeed();
            printTabs(m_nLevel);
            break;
    }
    m_eLastAction = STARTELEMENT;
    ++m_nLevel;
    // printf("<%s", ASCII_STRING(_rName));
    m_bCloseStartTag = sal_True;
}

//-----------------------------------------------------------------------------
void SAL_CALL OOpenNodeCallback::endElement( const ::rtl::OUString& _rName ) throw(sax::SAXException, RuntimeException)
{
    --m_nLevel;
    switch (m_eLastAction)
    {
        case STARTELEMENT:
            if (m_bCloseStartTag)
            {
                // printf("/>");
            }

            m_bCloseStartTag = sal_False;
            break;
        case ENDELEMENT:
            linefeed();
            printTabs(m_nLevel);
            // dont break
        case CHARACTERS:
            // printf("</%s>", ASCII_STRING(_rName));
            break;
    }
    m_eLastAction = ENDELEMENT;
}

//-----------------------------------------------------------------------------
void SAL_CALL OOpenNodeCallback::characters( const ::rtl::OUString& _rChars ) throw(sax::SAXException, RuntimeException)
{
    if (STARTELEMENT == m_eLastAction)
    {
        if (m_bCloseStartTag && _rChars.trim().getLength())
        {
            // printf(">");
            m_bCloseStartTag = sal_False;
        }
    }
    if (_rChars.trim().getLength() != 0)
    {
        // printf("%s", ASCII_STRING(_rChars));
        m_eLastAction = CHARACTERS;
    }
}

//-----------------------------------------------------------------------------
void OOpenNodeCallback::gotObjectId(const OUString &_nId)
{
    m_sNodeId = _nId;
    // printf("object id %i\n", m_nNodeId);
}

//-----------------------------------------------------------------------------
void OOpenNodeCallback::acknowledged(sal_Int32 _nTransId)
{
    // printf("acknowledged, transaction id : %i\n", _nTransId);
}

//-----------------------------------------------------------------------------
void OOpenNodeCallback::failed(sal_Int32 _nErrorCode)
{
    // printf("failed because of a connection error (%i)\n", _nErrorCode);
    m_rFinishCondition.set();
}

//-----------------------------------------------------------------------------
void OOpenNodeCallback::done(const StatusInfo& _rStatus)
{
    if (_rStatus.nCode)
    {
        printf("\n\ndone, but had an error : %s\n", ASCII_STRING(_rStatus.sMessage));
    }
    else
    {
        // printf("\n\nsuccessfully done\n", ASCII_STRING(_rStatus.sMessage));
    }

    m_rFinishCondition.set();
}

//=============================================================================
//= ONodeUpdater
//=============================================================================
class ONodeUpdater : public IDOMNodeDataProvider
{
    sal_Bool            m_bWriterLevel;
    sal_Bool m_bAdd;
public:
    ONodeUpdater(sal_Bool _bStartAtWriterLevel, sal_Bool bAdd) : m_bWriterLevel(_bStartAtWriterLevel), m_bAdd(bAdd) { }

    virtual void    writeNodeData(const Reference< sax::XDocumentHandler >& _rHandler);
};

//-----------------------------------------------------------------------------
void ONodeUpdater::writeNodeData(const Reference< sax::XDocumentHandler >& _rHandler)
{
    AttributeListImpl *pAttr = new AttributeListImpl;
    Reference< sax::XAttributeList > xEmptyAttrList = pAttr;

    pAttr = new AttributeListImpl;
    pAttr->addAttribute(UNI_STRING("type"),UNI_STRING("CDATA"),UNI_STRING("string"));
    Reference< sax::XAttributeList > xStringAttrList = pAttr;

    if (m_bWriterLevel)
        _rHandler->startElement(UNI_STRING("Writer"), xEmptyAttrList);
#ifdef USE_LAYOUT_NODE
    _rHandler->startElement(UNI_STRING("Layout"), xEmptyAttrList);
    _rHandler->startElement(UNI_STRING("TabStops"), xEmptyAttrList);
    if (m_bAdd)
        _rHandler->characters(UNI_STRING("0.90"));
    else
        _rHandler->characters(UNI_STRING("99.99"));

    _rHandler->endElement(UNI_STRING("TabStops"));
    _rHandler->endElement(UNI_STRING("Layout"));
#else
    // _rHandler->startElement(UNI_STRING("com.sun.star.office.Setup"), xEmptyAttrList);
    _rHandler->startElement(UNI_STRING("Modules"), xEmptyAttrList);
    _rHandler->startElement(UNI_STRING("StandFonts"), xEmptyAttrList);
    _rHandler->startElement(UNI_STRING("Standard"), xStringAttrList);
    _rHandler->startElement(UNI_STRING("value"), xEmptyAttrList);
    if (m_bAdd)
        _rHandler->characters(UNI_STRING("Arial"));
    else
        _rHandler->characters(UNI_STRING("Courier"));

    _rHandler->endElement(UNI_STRING("value"));
    _rHandler->endElement(UNI_STRING("Standard"));
    _rHandler->endElement(UNI_STRING("StandFonts"));
    _rHandler->endElement(UNI_STRING("Modules"));
    // _rHandler->endElement(UNI_STRING("com.sun.star.office.Setup"));

    _rHandler->ignorableWhitespace(OUString());
#endif
    if (m_bWriterLevel)
        _rHandler->endElement(UNI_STRING("Writer"));
}

//=============================================================================
//= OSessionListener
//=============================================================================
class OSessionListener : public ISessionListener
{
protected:
    oslInterlockedCount     m_refCount;

public:
    // ISessionListener
    virtual void    nodeUpdated(const ::rtl::OUString& _rNodePath);
    virtual void    nodeDeleted(const ::rtl::OUString& _rNodePath);
    virtual void    nodeAdded(const ::rtl::OUString& _rNodePath);

    // IInterface
    virtual void SAL_CALL acquire(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL release(  ) throw (::com::sun::star::uno::RuntimeException);
};

//-----------------------------------------------------------------------------
void OSessionListener::nodeUpdated(const ::rtl::OUString& _rNodePath)
{
    // printf("[listener] : a node was updated, node path : %s\n", ASCII_STRING(_rNodePath));
}

//-----------------------------------------------------------------------------
void OSessionListener::nodeDeleted(const ::rtl::OUString& _rNodePath)
{
    // printf("[listener] : a node was deleted, node path : %s\n", ASCII_STRING(_rNodePath));
}

//-----------------------------------------------------------------------------
void OSessionListener::nodeAdded(const ::rtl::OUString& _rNodePath)
{
    // printf("\n[listener] : a node was added, node path : %s", ASCII_STRING(_rNodePath));
}

//-----------------------------------------------------------------------------
void SAL_CALL OSessionListener::acquire(  ) throw (::com::sun::star::uno::RuntimeException)
{
    osl_incrementInterlockedCount(&m_refCount);
}

//-----------------------------------------------------------------------------
void SAL_CALL OSessionListener::release(  ) throw (::com::sun::star::uno::RuntimeException)
{
    if (!osl_decrementInterlockedCount(&m_refCount))
        delete this;
}

//=============================================================================
//=============================================================================

//=============================================================================
static ::rtl::OUString  sHost;
static sal_Int32        nPort;
static ::rtl::OUString  sRegistry = ::rtl::OUString::createFromAscii("applicat.rdb");

//=============================================================================
sal_Bool collectArgs(int argc, char * argv[])
{
    if (argc > 1)
    {
        sal_Char* pConnectTo = argv[1];
        sal_Char* pSeparator = strchr(pConnectTo, ':');
        if (pSeparator && (0 != *(pSeparator + 1)))
        {
            sHost = ::rtl::OUString(pConnectTo, pSeparator - pConnectTo, RTL_TEXTENCODING_ASCII_US);
            nPort = ::rtl::OUString::createFromAscii(pSeparator + 1).toInt32();

            if (argc > 2)
                sRegistry = ::rtl::OUString::createFromAscii(argv[2]);
            return sal_True;
        }
    }

    printf("cfgclient - registry server client test ...\n\r\n\r");
    printf("usage :\n\r");
    printf("    cfgclient <server>:<port> [<registry file>]\n\r\n\r");
    printf("    <server>        : machine to connect to\n\r");
    printf("    <port>          : port to connect to\n\r");
    printf("    <registry file> : (optional) registry to bootstrap from. defaulted to \"applicat.rdb\"\n\r\n\r");
    return sal_False;
}

//=============================================================================

#if (defined UNX) || (defined OS2)
void main( int argc, char * argv[] )
#else
void _cdecl main( int argc, char * argv[] )
#endif
{
    Reference< XMultiServiceFactory > xORB;
    try
    {
        xORB = createRegistryServiceFactory(sRegistry, ::rtl::OUString());
    }
    catch (Exception& e)
    {
        printf("could not bootstrap the services from %s\n\r", ASCII_STRING(sRegistry));
        printf("    (error message : %s)", ASCII_STRING(e.Message));
    }

    if (!xORB.is())
    {
        fprintf(stdout, "could not create the service factory !");
        return;
    }

    ORef< OSessionListener > xListener = new OSessionListener;
    {
/*
        ORemoteSession aSession(xORB);
        // --------- connect ----------
        sal_Bool bSuccess = aSession.connect(sHost, nPort, &aTimeout);
        if (!bSuccess)
        {
            printf("could not connect to the server (error : %i) ...", aSession.getConnectionError());
            return;
        }
*/

        void testSession(const Reference< XMultiServiceFactory > &xORB, bool bPrint);

        for (int i=0;i<100;i++)
        {
            TimeValue aStartTime, aEndTime;
            osl_getSystemTime(&aStartTime);
            testSession(xORB, false);
            osl_getSystemTime(&aEndTime);

            sal_Int32 nSeconds = aEndTime.Seconds - aStartTime.Seconds;
            sal_Int32 nNanoSec = aEndTime.Nanosec - aStartTime.Nanosec;
            if (nNanoSec < 0)
            {
                nNanoSec = 1000000000 - nNanoSec;
                nSeconds++;
            }

            cout << "Time: " << nSeconds << ". " << nNanoSec << endl;
        }
    }
}

void testSession(const Reference< XMultiServiceFactory > &xORB, bool bPrint)
{
    TimeValue aTimeout;
    aTimeout.Seconds = 5;
    aTimeout.Nanosec = 0;


    // create it .. and connect
    LocalSession aSession(xORB);

    // --------- openSession ----------

    OUString aRootPath = OUString::createFromAscii("f:/local/SRC598/configmgr/workben/local_io");
    // f:/local/SRC595/configmgr/workben/local_io);
    aSession.open(aRootPath);

    if (aSession.isOpen())
    {
        if (bPrint) printf("\nopened the session ...");
    }
    else
    {
        printf("\ncould not open the session ... exiting\n\n");
        return;
    }

    // aSession.setListener(xListener.getBodyPtr());

    OCondition aWaitForSomething;
    aWaitForSomething.reset();

    // --------- openNode ----------
    char* pWriterNode = "com.sun.star.Setup/Modules";
    if (bPrint) printf("\nsending an openNode request for %s ...\n", pWriterNode);
    ORef< OOpenNodeCallback > xOpenCallback = new OOpenNodeCallback(aWaitForSomething);
    aSession.openNode(UNI_STRING(pWriterNode), 2, xOpenCallback.getBodyPtr());

    aTimeout.Seconds = 30;
    switch (aWaitForSomething.wait(&aTimeout))
    {
    case ICondition::result_error:
        printf("error while waiting for the callback ... exiting\n\n");
        return;
    case ICondition::result_timeout:
        if (bPrint) printf("timed out ... exiting\n\n");
        return;
    }

    OUString sOpenedNode = xOpenCallback->getNodeId();

//         aSession.getNode(UNI_STRING("com.sun.star.Spreadsheet"), NULL);
//
//         // --------- getNode (1) ----------
//#ifdef USE_LAYOUT_NODE
//  char* pLayoutNode = "com.sun.star.office.Setup/Modules";
//#else
    char* pLayoutNode = "com.sun.star.Setup/Modules";
//#endif
//         printf("\nsending an getNode request for %s ...\n", pLayoutNode);
//         aWaitForSomething.reset();
//         aSession.getNode(UNI_STRING(pLayoutNode), new OOpenNodeCallback(aWaitForSomething));
//
//         switch (aWaitForSomething.wait(&aTimeout))
//         {
//             case ICondition::result_error:
//                 printf("error while waiting for the callback ... exiting\n\n");
//                 return;
//             case ICondition::result_timeout:
//                 printf("timed out ... exiting\n\n");
//                 return;
//         }

        // --------- addNode ----------
    if (bPrint) printf("\n\naddNode ....");
    ONodeUpdater aAddTabstops(sal_False, true);  // true = WRITER LEVEL
    aWaitForSomething.reset();
    aSession.addNode(sOpenedNode, UNI_STRING(pLayoutNode), &aAddTabstops, new OOpenNodeCallback(aWaitForSomething));
    switch (aWaitForSomething.wait(&aTimeout))
    {
    case ICondition::result_error:
        printf("error while waiting for the callback ... exiting\n\n");
        return;
    case ICondition::result_timeout:
        if (bPrint) printf("timed out ... exiting\n\n");
        return;
    }

    // --------- updateNode ----------
    if (bPrint) printf("\n\nokay, let's try an update ....\n");

    ONodeUpdater aUpdateWriter(sal_False, false);
    aWaitForSomething.reset();
    aSession.updateNode(sOpenedNode, UNI_STRING(pLayoutNode), &aUpdateWriter, new OOpenNodeCallback(aWaitForSomething));

    switch (aWaitForSomething.wait(&aTimeout))
    {
    case ICondition::result_error:
        printf("error while waiting for the callback ... exiting\n\n");
        return;
    case ICondition::result_timeout:
        if (bPrint) printf("timed out ... exiting\n\n");
        return;
    }

    // --------- deleteNode ----------
    char* pLayoutNode2 = "com.sun.star.Setup/Modules/StandFonts";

    if (bPrint) printf("\n\ndeleteNode ....");
    aWaitForSomething.reset();
    aSession.deleteNode(sOpenedNode, UNI_STRING(pLayoutNode2), new OOpenNodeCallback(aWaitForSomething));
    switch (aWaitForSomething.wait(&aTimeout))
    {
    case ICondition::result_error:
        printf("error while waiting for the callback ... exiting\n\n");
        return;
    case ICondition::result_timeout:
        if (bPrint) printf("timed out ... exiting\n\n");
        return;
    }

/*
  // --------- startListening ----------
  printf("\n\nadding a listener for the Layout node\n");
  Sequence< ::rtl::OUString > aNodesToListen(1);
  aNodesToListen[0] = UNI_STRING(pLayoutNode);
  aSession.startListening(aNodesToListen, NULL);

  printf("waiting 10 seconds ....\n\n");
  aWaitForSomething.reset();
  aTimeout.Seconds = 10;
  aWaitForSomething.wait(&aTimeout);
*/
    // --------- getNode (2) ----------
//         printf("\ndoing a new getNode for the Layout node ...\n");
//         aWaitForSomething.reset();
//         aSession.getNode(UNI_STRING(pLayoutNode), new OOpenNodeCallback(aWaitForSomething));
//         switch (aWaitForSomething.wait(&aTimeout))
//         {
//             case ICondition::result_error:
//                 printf("error while waiting for the callback ... exiting\n\n");
//                 return;
//             case ICondition::result_timeout:
//                 printf("timed out ... exiting\n\n");
//                 return;
//         }

    aSession.closeNode(sOpenedNode, NULL);
    aSession.close(NULL);

}

