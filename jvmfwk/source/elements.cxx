/*************************************************************************
 *
 *  $RCSfile: elements.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: jl $ $Date: 2004-04-26 15:52:14 $
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
#include "elements.hxx"
#include "osl/mutex.hxx"
#include "osl/file.hxx"
#include "fwkutil.hxx"
#include "libxmlutil.hxx"
#include "osl/thread.hxx"
#include <algorithm>
#include "libxml/parser.h"
#include "libxml/xpath.h"
#include "libxml/xpathinternals.h"

// #define NS_JAVA_FRAMEWORK "http://openoffice.org/2004/java/framework/1.0"
// #define NS_SCHEMA_INSTANCE "http://www.w3.org/2001/XMLSchema-instance"

namespace jfw
{

xmlNode* findChildNode(const xmlNode * pParent, const xmlChar* pName)
{
    xmlNode* ret = NULL;

    if (pParent)
    {
        xmlNode* cur = pParent->children;
        while (cur != NULL)
        {
            if (xmlStrcmp(cur->name, pName) == 0)
                break;
            cur = cur->next;
        }
        ret = cur;
    }
    return ret;
}

javaFrameworkError getElementUpdated(rtl::OString & sValue)
{
    javaFrameworkError errcode = JFW_E_NONE;
    //Prepare the xml document and context
    rtl::OString sSettingsPath = jfw::getVendorSettingsPath();
     jfw::CXmlDocPtr doc = xmlParseFile(sSettingsPath.getStr());
    if (doc == NULL)
    {
        OSL_ASSERT(0);
        return JFW_E_CONFIG_READWRITE;
    }
    jfw::CXPathContextPtr context = xmlXPathNewContext(doc);
    int reg = xmlXPathRegisterNs(context, (xmlChar*) "jf",
        (xmlChar*) NS_JAVA_FRAMEWORK);
    if (reg == -1)
        return JFW_E_ERROR;
    CXPathObjectPtr pathObj = xmlXPathEvalExpression(
        (xmlChar*)"/jf:javaSelection/jf:updated/text()", context);
    if (xmlXPathNodeSetIsEmpty(pathObj->nodesetval))
            return JFW_E_FORMAT_STORE;
    sValue = (sal_Char*) pathObj->nodesetval->nodeTab[0]->content;

    return errcode;
}

javaFrameworkError createUserSettingsDocument()
{
    javaFrameworkError ret = JFW_E_NONE;
    // check if javasettings.xml already exist
    rtl::OUString sURL = getUserSettingsURL();

    osl::DirectoryItem testFileItem;
    osl::File::RC fileError = osl::DirectoryItem::get(sURL, testFileItem);
    if (fileError == osl::FileBase::E_None)
        //file exist already
        return JFW_E_NONE;
    else if (fileError != osl::FileBase::E_NOENT)
        return JFW_E_ERROR;

    //javasettings.xml does not exist yet
    CXmlDocPtr doc = xmlNewDoc((xmlChar *)"1.0");
    if (! doc)
        return JFW_E_ERROR;
    //Create a comment
    xmlNewDocComment(
        doc, (xmlChar *) "This is a generated file. Do not alter this file!");

    //Create the root element and name spaces
    xmlNodePtr root =   xmlNewDocNode(
        doc, NULL, (xmlChar *) "java", (xmlChar *) "\n");
    if (root == NULL)
        return JFW_E_ERROR;
    if (xmlNewNs(root, (xmlChar *) NS_JAVA_FRAMEWORK,NULL) == NULL)
        return JFW_E_ERROR;
    if (xmlNewNs(root,(xmlChar*) NS_SCHEMA_INSTANCE,(xmlChar*)"xsi") == NULL)
        return JFW_E_ERROR;
    xmlDocSetRootElement(doc,  root);

    //Create a comment
    xmlNodePtr com = xmlNewComment(
        (xmlChar *) "This is a generated file. Do not alter this file!");
    if (com == NULL)
        return JFW_E_ERROR;

    if (xmlAddPrevSibling(root, com) == NULL)
        return JFW_E_ERROR;

    rtl::OString sSettingsPath = jfw::getUserSettingsPath();
    if (xmlSaveFormatFileEnc(
            sSettingsPath.getStr(), doc,"UTF-8", 1) == -1)
        return JFW_E_CONFIG_READWRITE;

    return ret;
}

javaFrameworkError createSettingsStructure(xmlDoc * document, bool * bNeedsSave)
{
    javaFrameworkError errcode = JFW_E_NONE;
    xmlNode * root = xmlDocGetRootElement(document);
    if (root == NULL)
        return JFW_E_ERROR;
    bool bFound = false;
    xmlNode * cur = root->children;
    while (cur != NULL)
    {
        if (xmlStrcmp(cur->name, (xmlChar*) "enabled") == 0)
        {
            bFound = true;
            break;
        }
        cur = cur->next;
    }
    if (bFound)
    {
        bNeedsSave = false;
        return errcode;
    }
    //We will modify this document
    *bNeedsSave = true;
    // Now we create the child elements ------------------
    //Get xsi:nil namespace
    xmlNs* nsXsi = xmlSearchNsByHref(
        document, root,(xmlChar*)  NS_SCHEMA_INSTANCE);

    //<classesDirectory/>
    xmlNode  * nodeCP = xmlNewTextChild(
        root,NULL, (xmlChar*) "classesDirectory", (xmlChar*) "");
    if (nodeCP == NULL)
        return JFW_E_ERROR;
    //add a new line
    xmlNode * nodeCrLf = xmlNewText((xmlChar*) "\n");
    xmlAddChild(root, nodeCrLf);

    //<enabled xsi:nil="true"
    xmlNode  * nodeEn = xmlNewTextChild(
        root,NULL, (xmlChar*) "enabled", (xmlChar*) "");
    if (nodeEn == NULL)
        return JFW_E_ERROR;
    xmlSetNsProp(nodeEn,nsXsi,(xmlChar*) "nil",(xmlChar*) "true");
    //add a new line
    nodeCrLf = xmlNewText((xmlChar*) "\n");
    xmlAddChild(root, nodeCrLf);

    //<userClassPath xsi:nil="true">
    xmlNode  * nodeUs = xmlNewTextChild(
        root,NULL, (xmlChar*) "userClassPath", (xmlChar*) "");
    if (nodeUs == NULL)
        return JFW_E_ERROR;
    xmlSetNsProp(nodeUs,nsXsi,(xmlChar*) "nil",(xmlChar*) "true");
    //add a new line
    nodeCrLf = xmlNewText((xmlChar*) "\n");
    xmlAddChild(root, nodeCrLf);

    //<vmParameters xsi:nil="true">
    xmlNode  * nodeVm = xmlNewTextChild(
        root,NULL, (xmlChar*) "vmParameters", (xmlChar*) "");
    if (nodeVm == NULL)
        return JFW_E_ERROR;
    xmlSetNsProp(nodeVm,nsXsi,(xmlChar*) "nil",(xmlChar*) "true");
    //add a new line
    nodeCrLf = xmlNewText((xmlChar*) "\n");
    xmlAddChild(root, nodeCrLf);

    //<jreLocations xsi:nil="true">
    xmlNode  * nodeJre = xmlNewTextChild(
        root,NULL, (xmlChar*) "jreLocations", (xmlChar*) "");
    if (nodeJre == NULL)
        return JFW_E_ERROR;
    xmlSetNsProp(nodeJre,nsXsi,(xmlChar*) "nil",(xmlChar*) "true");
    //add a new line
    nodeCrLf = xmlNewText((xmlChar*) "\n");
    xmlAddChild(root, nodeCrLf);

    //<javaInfo xsi:nil="true">
    xmlNode  * nodeJava = xmlNewTextChild(
        root,NULL, (xmlChar*) "javaInfo", (xmlChar*) "");
    if (nodeJava == NULL)
        return JFW_E_ERROR;
    xmlSetNsProp(nodeJava,nsXsi,(xmlChar*) "nil",(xmlChar*) "true");
    //add a new line
    nodeCrLf = xmlNewText((xmlChar*) "\n");
    xmlAddChild(root, nodeCrLf);

    //only copied during first time setup for the current user and client
    //machine
    errcode = copyShareSettings(document, root);

    return errcode;
}

javaFrameworkError copyShareSettings(xmlDoc * doc, xmlNode * userParent)
{
    javaFrameworkError errcode = JFW_E_NONE;
    CXPathContextPtr contextShare;
    CXPathObjectPtr pathObj;

    //check if there is a share/config/javasettings.xml
    rtl::OUString sShareSettings = getSharedSettingsURLNoPlatformSuffix();

    osl::DirectoryItem testFileItem;
    osl::File::RC fileError = osl::DirectoryItem::get(
        sShareSettings, testFileItem);
    if (fileError == osl::FileBase::E_NOENT)
        return JFW_E_NONE;
    if (fileError != osl::FileBase::E_None)
        //file exist already
        return JFW_E_ERROR;

    //Prepare access to share javasettings.xml
    rtl::OString sSettings = getSharedSettingsPathNoPlatformSuffix();
    CXmlDocPtr docShare = xmlParseFile(sSettings.getStr());
    if (docShare == NULL)
        return JFW_E_CONFIG_READWRITE;
    contextShare = xmlXPathNewContext(docShare);
    if (xmlXPathRegisterNs(contextShare, (xmlChar*) "jf",
        (xmlChar*) NS_JAVA_FRAMEWORK) == -1)
        return JFW_E_CONFIG_READWRITE;

    //copy <classesDirectory>
    rtl::OString sExpression= rtl::OString("//jf:classesDirectory[1]/text()");
    pathObj = xmlXPathEvalExpression((xmlChar*) sExpression.getStr(),
                                     contextShare);
    if ( ! pathObj || xmlXPathNodeSetIsEmpty(pathObj->nodesetval))
        return JFW_E_FORMAT_STORE;

    CXmlCharPtr sClasses = xmlNodeListGetString(
        docShare, pathObj->nodesetval->nodeTab[0], 1);

    xmlNode* userClasses =
        findChildNode(userParent, (xmlChar*) "classesDirectory");
    OSL_ASSERT(userClasses);

    xmlNodeSetContent(userClasses, sClasses);

    return errcode;
}
javaFrameworkError prepareSettingsDocument()
{
    javaFrameworkError errcode = JFW_E_NONE;
    if ((errcode = createUserSettingsDocument()) != JFW_E_NONE)
        return errcode;

    rtl::OString sSettings = getUserSettingsPath();
    CXmlDocPtr doc = xmlParseFile(sSettings.getStr());
    if (!doc)
        return JFW_E_CONFIG_READWRITE;

    bool bNeedsSave = false;
    errcode = createSettingsStructure(doc, & bNeedsSave);
    if (errcode != JFW_E_NONE)
        return errcode;
    if (bNeedsSave)
    {
        if (xmlSaveFormatFileEnc(
                sSettings.getStr(), doc,"UTF-8", 1) == -1)
            return JFW_E_CONFIG_READWRITE;
    }
    return errcode;
}

//====================================================================
VersionInfo::VersionInfo(): arVersions(NULL)
{
}

VersionInfo::~VersionInfo()
{
    delete [] arVersions;
}

void VersionInfo::addExcludeVersion(const rtl::OUString& sVersion)
{
    vecExcludeVersions.push_back(sVersion);
}

rtl_uString** VersionInfo::getExcludeVersions()
{
    osl::MutexGuard guard(getFwkMutex());
    if (arVersions != NULL)
        return arVersions;

    arVersions = new rtl_uString*[vecExcludeVersions.size()];
    int j=0;
    typedef std::vector<rtl::OUString>::const_iterator it;
    for (it i = vecExcludeVersions.begin(); i != vecExcludeVersions.end();
         i++, j++)
    {
        arVersions[j] = vecExcludeVersions[j].pData;
    }
    return arVersions;
}

sal_Int32 VersionInfo::getExcludeVersionSize()
{
    return vecExcludeVersions.size();
}
//==================================================================

CNodeJava::CNodeJava():
    m_bEnabled(sal_True), m_bEnabledModified(false),
    m_bUserClassPathModified(false), m_bJavaInfoModified(false),
    m_bVmParametersModified(false), m_bJRELocationsModified(false)
{
}


javaFrameworkError CNodeJava::loadFromSettings()
{
    javaFrameworkError errcode = JFW_E_NONE;
    // share settings may not be given
    errcode = loadShareSettings();
    OSL_ASSERT(errcode == JFW_E_NONE);
    if (errcode == JFW_E_NONE)
        errcode = loadUserSettings();
    return errcode;
}


javaFrameworkError CNodeJava::loadUserSettings()
{
    javaFrameworkError errcode = JFW_E_NONE;
    CXmlDocPtr docUser;

    javaFrameworkError err = prepareSettingsDocument();

    //Read the user elements
    rtl::OString sSettingsPath = jfw::getUserSettingsPath();
    //There must not be a share settings file
    docUser = xmlParseFile(sSettingsPath.getStr());
    if (docUser == NULL)
        return JFW_E_CONFIG_READWRITE;

    xmlNode * cur = xmlDocGetRootElement(docUser);
    if (cur == NULL || cur->children == NULL)
        return JFW_E_FORMAT_STORE;

    cur = cur->children;
    while (cur != NULL)
    {
        if (xmlStrcmp(cur->name, (xmlChar*) "enabled") == 0)
        {
            //only overwrite share settings if xsi:nil="false"
            CXmlCharPtr sNil = xmlGetNsProp(
                cur, (xmlChar*) "nil", (xmlChar*) NS_SCHEMA_INSTANCE);
            if (sNil == NULL)
            {
                OSL_ASSERT(0);
                return JFW_E_FORMAT_STORE;
            }
            if (xmlStrcmp(sNil, (xmlChar*) "false") == 0)
            {
                CXmlCharPtr sEnabled = xmlNodeListGetString(
                    docUser, cur->children, 1);
                if (xmlStrcmp(sEnabled, (xmlChar*) "true") == 0)
                    m_bEnabled = sal_True;
                else if (xmlStrcmp(sEnabled, (xmlChar*) "false") == 0)
                    m_bEnabled = sal_False;
                else
                    m_bEnabled = sal_True;
            }
        }
        else if (xmlStrcmp(cur->name, (xmlChar*) "classesDirectory") == 0)
        {
            CXmlCharPtr sCls = xmlNodeListGetString(
                docUser, cur->children, 1);
            m_sClassesDirectory = sCls;
        }
        else if (xmlStrcmp(cur->name, (xmlChar*) "userClassPath") == 0)
        {
            CXmlCharPtr sNil = xmlGetNsProp(
                cur, (xmlChar*) "nil", (xmlChar*) NS_SCHEMA_INSTANCE);
            if (sNil == NULL)
            {
                OSL_ASSERT(0);
                return JFW_E_FORMAT_STORE;
            }
            if (xmlStrcmp(sNil, (xmlChar*) "false") == 0)
            {
                CXmlCharPtr sUser = xmlNodeListGetString(
                    docUser, cur->children, 1);
                m_sUserClassPath = sUser;
            }
        }
        else if (xmlStrcmp(cur->name, (xmlChar*) "javaInfo") == 0)
        {
            CXmlCharPtr sNil = xmlGetNsProp(
                cur, (xmlChar*) "nil", (xmlChar*) NS_SCHEMA_INSTANCE);
            if (sNil == NULL)
            {
                OSL_ASSERT(0);
                return JFW_E_FORMAT_STORE;
            }
            if (xmlStrcmp(sNil, (xmlChar*) "false") == 0)
            {
                m_aInfo.loadFromNode(docUser, cur);
            }
        }
        else if (xmlStrcmp(cur->name, (xmlChar*) "vmParameters") == 0)
        {
            CXmlCharPtr sNil = xmlGetNsProp(
                cur, (xmlChar*) "nil", (xmlChar*) NS_SCHEMA_INSTANCE);
            if (sNil == NULL)
            {
                OSL_ASSERT(0);
                return JFW_E_FORMAT_STORE;
            }
            if (xmlStrcmp(sNil, (xmlChar*) "false") == 0)
            {
                //throw away share settings
                m_arVmParameters.clear();
                xmlNode * pOpt = cur->children;
                while (pOpt != NULL)
                {
                    if (xmlStrcmp(pOpt->name, (xmlChar*) "param") == 0)
                    {
                        CXmlCharPtr sOpt = xmlNodeListGetString(
                            docUser, pOpt->children, 1);
                        m_arVmParameters.push_back(sOpt);
                    }
                    pOpt = pOpt->next;
                }
            }
        }
        else if (xmlStrcmp(cur->name, (xmlChar*) "jreLocations") == 0)
        {
            CXmlCharPtr sNil = xmlGetNsProp(
                cur, (xmlChar*) "nil", (xmlChar*) NS_SCHEMA_INSTANCE);
            if (sNil == NULL)
            {
                OSL_ASSERT(0);
                return JFW_E_FORMAT_STORE;
            }
            if (xmlStrcmp(sNil, (xmlChar*) "false") == 0)
            {
                //throw away share settings
                m_arJRELocations.clear();
                xmlNode * pLoc = cur->children;
                while (pLoc != NULL)
                {
                    if (xmlStrcmp(pLoc->name, (xmlChar*) "location") == 0)
                    {
                        CXmlCharPtr sLoc = xmlNodeListGetString(
                            docUser, pLoc->children, 1);
                        m_arJRELocations.push_back(sLoc);
                    }
                    pLoc = pLoc->next;
                }
            }
        }

        cur = cur->next;
    }
    return errcode;
}

javaFrameworkError CNodeJava::loadShareSettings()
{
    javaFrameworkError errcode = JFW_E_NONE;
    CXmlDocPtr docShare;

    //Read the share elements, do not heed the nil attributes
    rtl::OString sSettingsPath = jfw::getSharedSettingsPath();
    //There must not be a share settings file
    docShare = xmlParseFile(sSettingsPath.getStr());
    if (docShare == NULL)
        return JFW_E_NONE;

    xmlNode * cur = xmlDocGetRootElement(docShare);
    if (cur == NULL)
    {
        OSL_ASSERT(cur);
        return JFW_E_FORMAT_STORE;
    }
    if (cur->children == NULL)
        return JFW_E_NONE;
    cur = cur->children;
    while (cur != NULL)
    {
        if (xmlStrcmp(cur->name, (xmlChar*) "enabled") == 0)
        {
            CXmlCharPtr sEnabled = xmlNodeListGetString(
                docShare, cur->children, 1);
            if (xmlStrcmp(sEnabled, (xmlChar*) "true") == 0)
                m_bEnabled = sal_True;
            else if (xmlStrcmp(sEnabled, (xmlChar*) "false") == 0)
                m_bEnabled = sal_False;
            else
                m_bEnabled = sal_True;
        }
        else if (xmlStrcmp(cur->name, (xmlChar*) "classesDirectory") == 0)
        {
            CXmlCharPtr sCls = xmlNodeListGetString(
                docShare, cur->children, 1);
            m_sClassesDirectory = sCls;
        }
        else if (xmlStrcmp(cur->name, (xmlChar*) "userClassPath") == 0)
        {
            CXmlCharPtr sUser = xmlNodeListGetString(
                docShare, cur->children, 1);
            m_sUserClassPath = sUser;
        }
        else if (xmlStrcmp(cur->name, (xmlChar*) "javaInfo") == 0)
        {
            m_aInfo.loadFromNode(docShare, cur);
        }
        else if (xmlStrcmp(cur->name, (xmlChar*) "vmParameters") == 0)
        {
            xmlNode * pOpt = cur->children;
            while (pOpt != NULL)
            {
                if (xmlStrcmp(pOpt->name, (xmlChar*) "param") == 0)
                {
                    CXmlCharPtr sOpt = xmlNodeListGetString(
                        docShare, pOpt->children, 1);
                    m_arVmParameters.push_back(sOpt);
                }
                    pOpt = pOpt->next;
            }
        }
        else if (xmlStrcmp(cur->name, (xmlChar*) "jreLocations") == 0)
        {
            xmlNode * pLoc = cur->children;
            while (pLoc != NULL)
            {
                if (xmlStrcmp(pLoc->name, (xmlChar*) "location") == 0)
                {
                    CXmlCharPtr sLoc = xmlNodeListGetString(
                        docShare, pLoc->children, 1);
                    m_arJRELocations.push_back(sLoc);
                }
                pLoc = pLoc->next;
            }
        }

        cur = cur->next;
    }
    return errcode;
}

javaFrameworkError CNodeJava::writeSettings() const
{
    javaFrameworkError errcode = JFW_E_NONE;
    CXmlDocPtr docUser;
    CXPathContextPtr contextUser;
    CXPathObjectPtr pathObj;

    javaFrameworkError err = prepareSettingsDocument();
    //Read the user elements
    rtl::OString sSettingsPath = jfw::getUserSettingsPath();
    docUser = xmlParseFile(sSettingsPath.getStr());
    if (docUser == NULL)
        return JFW_E_CONFIG_READWRITE;
    contextUser = xmlXPathNewContext(docUser);
    if (xmlXPathRegisterNs(contextUser, (xmlChar*) "jf",
        (xmlChar*) NS_JAVA_FRAMEWORK) == -1)
        return JFW_E_CONFIG_READWRITE;

    xmlNode * root = xmlDocGetRootElement(docUser);
    //Get xsi:nil namespace
    xmlNs* nsXsi = xmlSearchNsByHref(docUser,
                             root,
                             (xmlChar*)  NS_SCHEMA_INSTANCE);

    //set the <enabled> element
    //The element must exist
    if (m_bEnabledModified)
    {
        rtl::OString sExpression= rtl::OString(
            "/jf:java/jf:enabled");
        pathObj = xmlXPathEvalExpression((xmlChar*) sExpression.getStr(),
                                         contextUser);
        if ( ! pathObj || xmlXPathNodeSetIsEmpty(pathObj->nodesetval))
            return JFW_E_FORMAT_STORE;

        xmlNode * nodeEnabled = pathObj->nodesetval->nodeTab[0];
        xmlSetNsProp(nodeEnabled,
                     nsXsi,
                     (xmlChar*) "nil",
                     (xmlChar*) "false");

        if (m_bEnabled == sal_True)
            xmlNodeSetContent(nodeEnabled,(xmlChar*) "true");
        else
            xmlNodeSetContent(nodeEnabled,(xmlChar*) "false");
    }

    //set the <userClassPath> element
    //The element must exist
    if (m_bUserClassPathModified)
    {
        rtl::OString sExpression= rtl::OString(
            "/jf:java/jf:userClassPath");
        pathObj = xmlXPathEvalExpression((xmlChar*) sExpression.getStr(),
                                         contextUser);
        if ( ! pathObj || xmlXPathNodeSetIsEmpty(pathObj->nodesetval))
            return JFW_E_FORMAT_STORE;

        xmlNode * nodeEnabled = pathObj->nodesetval->nodeTab[0];
        xmlSetNsProp(nodeEnabled, nsXsi, (xmlChar*) "nil",(xmlChar*) "false");

        rtl::OString osUserCP =
            rtl::OUStringToOString(m_sUserClassPath, osl_getThreadTextEncoding());
        xmlNodeSetContent(nodeEnabled,(xmlChar*) osUserCP.getStr());
    }

    //set <javaInfo> element
    if (m_bJavaInfoModified)
    {
        rtl::OString sExpression= rtl::OString(
            "/jf:java/jf:javaInfo");
        pathObj = xmlXPathEvalExpression((xmlChar*) sExpression.getStr(),
                                                contextUser);
        if ( ! pathObj || xmlXPathNodeSetIsEmpty(pathObj->nodesetval))
            return JFW_E_FORMAT_STORE;

        errcode = m_aInfo.writeToNode(
            docUser, pathObj->nodesetval->nodeTab[0]);
        if (errcode != JFW_E_NONE)
            return errcode;
    }

    //set <vmParameters> element
    if (m_bVmParametersModified)
    {
        rtl::OString sExpression= rtl::OString(
            "/jf:java/jf:vmParameters");
        pathObj = xmlXPathEvalExpression((xmlChar*) sExpression.getStr(),
                                         contextUser);
        if ( ! pathObj || xmlXPathNodeSetIsEmpty(pathObj->nodesetval))
            return JFW_E_FORMAT_STORE;
        xmlNode* vmParameters = pathObj->nodesetval->nodeTab[0];
        //set xsi:nil = false;
        xmlSetNsProp(vmParameters, nsXsi,(xmlChar*) "nil",
                     (xmlChar*) "false");

        //remove option elements
        xmlNode* cur = vmParameters->children;
        while (cur != NULL)
        {
            xmlNode* lastNode = cur;
            cur = cur->next;
            xmlUnlinkNode(lastNode);
            xmlFreeNode(lastNode);
        }
        //add a new line after <vmParameters>
        if (m_arVmParameters.size() > 0)
        {
            xmlNode * nodeCrLf = xmlNewText((xmlChar*) "\n");
            xmlAddChild(vmParameters, nodeCrLf);
        }

        typedef std::vector<rtl::OString>::const_iterator cit;
        for (cit i = m_arVmParameters.begin(); i != m_arVmParameters.end(); i++)
        {
            xmlNewTextChild(vmParameters, NULL, (xmlChar*) "param",
                            (xmlChar*) i->getStr());
            //add a new line
            xmlNode * nodeCrLf = xmlNewText((xmlChar*) "\n");
            xmlAddChild(vmParameters, nodeCrLf);
        }
    }

    //set <jreLocations> element
    if (m_bJRELocationsModified)
    {
        rtl::OString sExpression= rtl::OString(
            "/jf:java/jf:jreLocations");
        pathObj = xmlXPathEvalExpression((xmlChar*) sExpression.getStr(),
                                         contextUser);
        if ( ! pathObj || xmlXPathNodeSetIsEmpty(pathObj->nodesetval))
            return JFW_E_FORMAT_STORE;
        xmlNode* jreLocationsNode = pathObj->nodesetval->nodeTab[0];
        //set xsi:nil = false;
        xmlSetNsProp(jreLocationsNode, nsXsi,(xmlChar*) "nil",
                     (xmlChar*) "false");

        //remove option elements
        xmlNode* cur = jreLocationsNode->children;
        while (cur != NULL)
        {
            xmlNode* lastNode = cur;
            cur = cur->next;
            xmlUnlinkNode(lastNode);
            xmlFreeNode(lastNode);
        }
        //add a new line after <vmParameters>
        if (m_arJRELocations.size() > 0)
        {
            xmlNode * nodeCrLf = xmlNewText((xmlChar*) "\n");
            xmlAddChild(jreLocationsNode, nodeCrLf);
        }

        typedef std::vector<rtl::OString>::const_iterator cit;
        for (cit i = m_arJRELocations.begin(); i != m_arJRELocations.end(); i++)
        {
            xmlNewTextChild(jreLocationsNode, NULL, (xmlChar*) "location",
                            (xmlChar*) i->getStr());
            //add a new line
            xmlNode * nodeCrLf = xmlNewText((xmlChar*) "\n");
            xmlAddChild(jreLocationsNode, nodeCrLf);
        }
    }

//    sExpression =
    if (xmlSaveFormatFile(sSettingsPath.getStr(), docUser, 1) == -1)
        return JFW_E_CONFIG_READWRITE;

    return JFW_E_NONE;
}

void CNodeJava::setEnabled(sal_Bool bEnabled)
{
    m_bEnabled = bEnabled;
    m_bEnabledModified = true;
}

sal_Bool CNodeJava::getEnabled() const
{
    return m_bEnabled;
}

void CNodeJava::setUserClassPath(const rtl::OUString & sClassPath)
{
    m_sUserClassPath = sClassPath;
    m_bUserClassPathModified = true;
}

rtl::OUString const &  CNodeJava::getUserClassPath() const
{
    return m_sUserClassPath;
}

void CNodeJava::setJavaInfo(const JavaInfo * pInfo)

{

    m_aInfo.bNil = false;
//    m_aInfo.sAttrVendorUpdate = sVendorUpdate;


    if (pInfo != NULL)
    {
        m_aInfo.m_bEmptyNode = false;
        m_aInfo.sVendor = pInfo->sVendor;
        m_aInfo.sLocation = pInfo->sLocation;
        m_aInfo.sVersion = pInfo->sVersion;
        m_aInfo.nFeatures = pInfo->nFeatures;
        m_aInfo.nRequirements = pInfo->nRequirements;
        m_aInfo.arVendorData = pInfo->arVendorData;
    }
    else
    {
        m_aInfo.m_bEmptyNode = true;
        rtl::OUString sEmpty;
        m_aInfo.sVendor = sEmpty;
        m_aInfo.sLocation = sEmpty;
        m_aInfo.sVersion = sEmpty;
        m_aInfo.nFeatures = 0;
        m_aInfo.nRequirements = 0;
        m_aInfo.arVendorData = rtl::ByteSequence();
    }
    m_bJavaInfoModified = true;
}

JavaInfo * CNodeJava::getJavaInfo() const
{
    return m_aInfo.makeJavaInfo();
}

rtl::OString const & CNodeJava::getJavaInfoAttrVendorUpdate() const
{
    return m_aInfo.sAttrVendorUpdate;
}

const std::vector<rtl::OString> & CNodeJava::getVmParameters() const
{
    return m_arVmParameters;
}

void CNodeJava::setVmParameters(rtl_uString * * arOptions, sal_Int32 size)
{
    OSL_ASSERT( !(arOptions == 0 && size != 0));
    m_arVmParameters.clear();
    if (arOptions != NULL)
    {
        for (int i  = 0; i < size; i++)
        {
            const rtl::OUString usOption = (rtl_uString*) arOptions[i];
            rtl::OString osOption = rtl::OUStringToOString(
                usOption, RTL_TEXTENCODING_UTF8);
            m_arVmParameters.push_back(osOption);
        }
    }
    m_bVmParametersModified = true;
}

void CNodeJava::getVmParametersArray(
    rtl_uString *** parParams, sal_Int32 * size) const
{
    osl::MutexGuard guard(getFwkMutex());
    OSL_ASSERT(parParams != NULL && size != NULL);

    *parParams = (rtl_uString **)
        rtl_allocateMemory(sizeof(rtl_uString*) * m_arVmParameters.size());
    if (*parParams == NULL)
        return;

    int j=0;
    typedef std::vector<rtl::OString>::const_iterator it;
    for (it i = m_arVmParameters.begin(); i != m_arVmParameters.end();
         i++, j++)
    {
        rtl::OUString sParam =
            rtl::OStringToOUString(*i, RTL_TEXTENCODING_UTF8);
        (*parParams)[j] = sParam.pData;
        rtl_uString_acquire(sParam.pData);
    }
    *size = m_arVmParameters.size();
}


void CNodeJava::setJRELocations(rtl_uString  * * arLocations, sal_Int32 size)
{
    OSL_ASSERT( !(arLocations == 0 && size != 0));
    m_arJRELocations.clear();
    if (arLocations != NULL)
    {
        for (int i  = 0; i < size; i++)
        {
            const rtl::OUString & usLocation = (rtl_uString*) arLocations[i];
            rtl::OString osLocation = rtl::OUStringToOString(
                usLocation, RTL_TEXTENCODING_UTF8);
            //only add the path if not already present
            std::vector<rtl::OString>::const_iterator it =
                std::find(m_arJRELocations.begin(), m_arJRELocations.end(),
                          osLocation);
            if (it == m_arJRELocations.end())
                m_arJRELocations.push_back(osLocation);
        }
    }
    m_bJRELocationsModified = true;
}

void CNodeJava::addJRELocation(rtl_uString * sLocation)
{
    OSL_ASSERT( sLocation);

    const rtl::OUString & usLocation = sLocation;
    rtl::OString osLocation = rtl::OUStringToOString(
        usLocation, RTL_TEXTENCODING_UTF8);
    //only add the path if not already present
    std::vector<rtl::OString>::const_iterator it =
        std::find(m_arJRELocations.begin(), m_arJRELocations.end(),
                  osLocation);
    if (it == m_arJRELocations.end())
        m_arJRELocations.push_back(osLocation);

    m_bJRELocationsModified = true;
}

const std::vector<rtl::OString> & CNodeJava::getJRELocations() const
{
    return m_arJRELocations;
}

void CNodeJava::getJRELocations(
    rtl_uString *** parLocations, sal_Int32 * size) const
{
    osl::MutexGuard guard(getFwkMutex());
    OSL_ASSERT(parLocations != NULL && size != NULL);

    *parLocations = (rtl_uString **)
        rtl_allocateMemory(sizeof(rtl_uString*) * m_arJRELocations.size());
    if (*parLocations == NULL)
        return;

    int j=0;
    typedef std::vector<rtl::OString>::const_iterator it;
    for (it i = m_arJRELocations.begin(); i != m_arJRELocations.end();
         i++, j++)
    {
        rtl::OUString sLocation =
            rtl::OStringToOUString(*i, RTL_TEXTENCODING_UTF8);
        (*parLocations)[j] = sLocation.pData;
        rtl_uString_acquire(sLocation.pData);
    }
    *size = m_arJRELocations.size();
}

//=====================================================================
CNodeJavaInfo::CNodeJavaInfo() :
    nFeatures(0), nRequirements(0), bNil(true), m_bEmptyNode(false)
{
}

CNodeJavaInfo::~CNodeJavaInfo()
{
}

CNodeJavaInfo::CNodeJavaInfo(const JavaInfo * pInfo)
{
    if (pInfo != NULL)
    {
        m_bEmptyNode = false;
//        sAttrVendorUpdate = sUpdated;
        sVendor = pInfo->sVendor;
        sLocation = pInfo->sLocation;
        sVersion = pInfo->sVersion;
        nFeatures = pInfo->nFeatures;
        nRequirements = pInfo->nRequirements;
        arVendorData = pInfo->arVendorData;
    }
}
javaFrameworkError CNodeJavaInfo::loadFromNode(xmlDoc * pDoc, xmlNode * pJavaInfo)
{
    javaFrameworkError errcode = JFW_E_NONE;
    OSL_ASSERT(pJavaInfo && pDoc);
    if (pJavaInfo->children == NULL)
        return JFW_E_NONE;
    //Get the xsi:nil attribute;
    CXmlCharPtr sNil = xmlGetNsProp(
        pJavaInfo, (xmlChar*) "nil", (xmlChar*) NS_SCHEMA_INSTANCE);
    if ( ! sNil)
        return JFW_E_FORMAT_STORE;
    if (xmlStrcmp(sNil, (xmlChar*) "true") == 0)
        bNil = true;
    else if (xmlStrcmp(sNil, (xmlChar*) "false") == 0)
        bNil = false;
    else
        return JFW_E_FORMAT_STORE;
    if (bNil == true)
        return JFW_E_NONE;

    xmlNode * cur = pJavaInfo->children;

    while (cur != NULL)
    {
        if (xmlStrcmp(cur->name, (xmlChar*) "vendor") == 0)
        {
            CXmlCharPtr xmlVendor = xmlNodeListGetString(
                pDoc, cur->children, 1);
            if (! xmlVendor)
                return JFW_E_NONE;
            sVendor = xmlVendor;
        }
        else if (xmlStrcmp(cur->name, (xmlChar*) "location") == 0)
        {
            CXmlCharPtr xmlLocation = xmlNodeListGetString(
                pDoc, cur->children, 1);
            sLocation = xmlLocation;
        }
        else if (xmlStrcmp(cur->name, (xmlChar*) "version") == 0)
        {
            CXmlCharPtr xmlVersion = xmlNodeListGetString(
                pDoc, cur->children, 1);
            sVersion = xmlVersion;
        }
        else if (xmlStrcmp(cur->name, (xmlChar*) "features")== 0)
        {
            CXmlCharPtr xmlFeatures = xmlNodeListGetString(
                    pDoc, cur->children, 1);
            rtl::OUString sFeatures = xmlFeatures;
            nFeatures = sFeatures.toInt64(16);
        }
        else if (xmlStrcmp(cur->name, (xmlChar*) "requirements") == 0)
        {
            CXmlCharPtr xmlRequire = xmlNodeListGetString(
                pDoc, cur->children, 1);
            rtl::OUString sRequire = xmlRequire;
            nRequirements = sRequire.toInt64(16);
        }
        else if (xmlStrcmp(cur->name, (xmlChar*) "vendorData") == 0)
        {
            CXmlCharPtr xmlData = xmlNodeListGetString(
                pDoc, cur->children, 1);
            xmlChar* _data = (xmlChar*) xmlData;
            if (_data)
            {
                rtl::ByteSequence seq((sal_Int8*) _data, strlen((char*)_data));
                arVendorData = decodeBase16(seq);
            }
        }
        cur = cur->next;
    }

    //Get the javainfo attributes
    CXmlCharPtr sVendorUpdate = xmlGetProp(pJavaInfo,
                                    (xmlChar*) "vendorUpdate");
    if ( ! sVendorUpdate)
        return JFW_E_FORMAT_STORE;
    sAttrVendorUpdate = sVendorUpdate;


    return errcode;
}


javaFrameworkError CNodeJavaInfo::writeToNode(xmlDoc* pDoc,
                                              xmlNode* pJavaInfoNode) const

{
    OSL_ASSERT(pJavaInfoNode && pDoc);
    javaFrameworkError errcode = JFW_E_NONE;
    //write the attribute vendorSettings

    //creates the attribute if necessary
    rtl::OString sUpdated;
    errcode = getElementUpdated(sUpdated);
    if (errcode != JFW_E_NONE)
        return errcode;
    xmlSetProp(pJavaInfoNode, (xmlChar*)"vendorUpdate",
                   (xmlChar*) sUpdated.getStr());

    //Set xsi:nil in javaInfo element to false
    //the xmlNs pointer must not be destroyed
    xmlNs* nsXsi = xmlSearchNsByHref((xmlDoc*) pDoc,
                             pJavaInfoNode,
                             (xmlChar*)  NS_SCHEMA_INSTANCE);

    xmlSetNsProp(pJavaInfoNode,
                 nsXsi,
                 (xmlChar*) "nil",
                 (xmlChar*) "false");

    //Delete the children of JavaInfo
    xmlNode* cur = pJavaInfoNode->children;
    while (cur != NULL)
    {
        xmlNode* lastNode = cur;
        cur = cur->next;
        xmlUnlinkNode(lastNode);
        xmlFreeNode(lastNode);
    }

    //If the JavaInfo was set with an empty value,
    //then we are done.
    if (m_bEmptyNode)
        return errcode;

    //add a new line after <javaInfo>
    xmlNode * nodeCrLf = xmlNewText((xmlChar*) "\n");
    xmlAddChild(pJavaInfoNode, nodeCrLf);

    //Create the vendor element
    rtl::OString osVendor = rtl::OUStringToOString(
        sVendor, osl_getThreadTextEncoding());
    xmlNewTextChild(pJavaInfoNode, NULL, (xmlChar*) "vendor",
                    (xmlChar*) osVendor.getStr());
    //add a new line for better readability
    nodeCrLf = xmlNewText((xmlChar*) "\n");
    xmlAddChild(pJavaInfoNode, nodeCrLf);

    //Create the location element
    rtl::OString osLocation = rtl::OUStringToOString(
        sLocation, osl_getThreadTextEncoding());

    xmlNewTextChild(pJavaInfoNode, NULL, (xmlChar*) "location",
                    (xmlChar*) osLocation.getStr());
    //add a new line for better readability
    nodeCrLf = xmlNewText((xmlChar*) "\n");
    xmlAddChild(pJavaInfoNode, nodeCrLf);

    //Create the version element
    rtl::OString osVersion = rtl::OUStringToOString(
        sVersion, osl_getThreadTextEncoding());

    xmlNewTextChild(pJavaInfoNode, NULL, (xmlChar*) "version",
                    (xmlChar*) osVersion.getStr());
    //add a new line for better readability
    nodeCrLf = xmlNewText((xmlChar*) "\n");
    xmlAddChild(pJavaInfoNode, nodeCrLf);

    //Create the features element
    rtl::OUString usFeatures = rtl::OUString::valueOf(
        (sal_Int64)nFeatures, 16);
    rtl::OString sFeatures = rtl::OUStringToOString(
        usFeatures, osl_getThreadTextEncoding());

    xmlNewTextChild(pJavaInfoNode, NULL, (xmlChar*) "features",
                    (xmlChar*) sFeatures.getStr());
    //add a new line for better readability
    nodeCrLf = xmlNewText((xmlChar*) "\n");
    xmlAddChild(pJavaInfoNode, nodeCrLf);


    //Create the requirements element
    rtl::OUString usRequirements = rtl::OUString::valueOf(
        (sal_Int64) nRequirements, 16);
    rtl::OString sRequirements = rtl::OUStringToOString(
        usRequirements, osl_getThreadTextEncoding());

    xmlNewTextChild(pJavaInfoNode, NULL, (xmlChar*) "requirements",
                    (xmlChar*) sRequirements.getStr());
    //add a new line for better readability
    nodeCrLf = xmlNewText((xmlChar*) "\n");
    xmlAddChild(pJavaInfoNode, nodeCrLf);


    //Create the features element
    rtl::ByteSequence data = encodeBase16(arVendorData);
    xmlNode* dataNode = xmlNewChild(pJavaInfoNode, NULL,
                                    (xmlChar*) "vendorData",
                                    (xmlChar*) "");
    xmlNodeSetContentLen(dataNode,
                         (xmlChar*) data.getArray(), data.getLength());
    //add a new line for better readability
    nodeCrLf = xmlNewText((xmlChar*) "\n");
    xmlAddChild(pJavaInfoNode, nodeCrLf);

    return JFW_E_NONE;
}

JavaInfo * CNodeJavaInfo::makeJavaInfo() const
{
    if (bNil == true)
        return NULL;
    JavaInfo * pInfo = (JavaInfo*) rtl_allocateMemory(sizeof JavaInfo);
    if (pInfo == NULL)
        return NULL;
    memset(pInfo, 0, sizeof JavaInfo);
    pInfo->sVendor = sVendor.pData;
    rtl_uString_acquire(pInfo->sVendor);
    pInfo->sLocation = sLocation.pData;
    rtl_uString_acquire(pInfo->sLocation);
    pInfo->sVersion = sVersion.pData;
    rtl_uString_acquire(pInfo->sVersion);
    pInfo->nFeatures = nFeatures;
    pInfo->nRequirements = nRequirements;
    pInfo->arVendorData = arVendorData.getHandle();
    rtl_byte_sequence_acquire(pInfo->arVendorData);
    return pInfo;
}
}
