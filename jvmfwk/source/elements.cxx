/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "elements.hxx"
#include "osl/mutex.hxx"
#include "osl/file.hxx"
#include "fwkutil.hxx"
#include "fwkbase.hxx"
#include "framework.hxx"
#include "libxmlutil.hxx"
#include "osl/thread.hxx"
#include <algorithm>
#include "libxml/parser.h"
#include "libxml/xpath.h"
#include "libxml/xpathInternals.h"
#include "rtl/bootstrap.hxx"
#include "boost/optional.hpp"
#include <string.h>


using namespace osl;
namespace jfw
{

rtl::OString getElement(::rtl::OString const & docPath,
                        xmlChar const * pathExpression, bool bThrowIfEmpty)
{
    //Prepare the xml document and context
    OSL_ASSERT(!docPath.isEmpty());
     jfw::CXmlDocPtr doc(xmlParseFile(docPath.getStr()));
    if (doc == NULL)
        throw FrameworkException(
            JFW_E_ERROR,
            rtl::OString("[Java framework] Error in function getElement "
                         "(elements.cxx)"));

    jfw::CXPathContextPtr context(xmlXPathNewContext(doc));
    if (xmlXPathRegisterNs(context, (xmlChar*) "jf",
        (xmlChar*) NS_JAVA_FRAMEWORK) == -1)
        throw FrameworkException(
            JFW_E_ERROR,
            rtl::OString("[Java framework] Error in function getElement "
                         "(elements.cxx)"));

    CXPathObjectPtr pathObj;
    pathObj = xmlXPathEvalExpression(pathExpression, context);
    rtl::OString sValue;
    if (xmlXPathNodeSetIsEmpty(pathObj->nodesetval))
    {
        if (bThrowIfEmpty)
            throw FrameworkException(
                JFW_E_ERROR,
                rtl::OString("[Java framework] Error in function getElement "
                             "(elements.cxx)"));
    }
    else
    {
        sValue = (sal_Char*) pathObj->nodesetval->nodeTab[0]->content;
    }
    return sValue;
}

rtl::OString getElementUpdated()
{
    return getElement(jfw::getVendorSettingsPath(),
                      (xmlChar*)"/jf:javaSelection/jf:updated/text()", true);
}

void createSettingsStructure(xmlDoc * document, bool * bNeedsSave)
{
    rtl::OString sExcMsg("[Java framework] Error in function createSettingsStructure "
                         "(elements.cxx).");
    xmlNode * root = xmlDocGetRootElement(document);
    if (root == NULL)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);
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
        *bNeedsSave = false;
        return;
    }
    //We will modify this document
    *bNeedsSave = true;
    // Now we create the child elements ------------------
    //Get xsi:nil namespace
    xmlNs* nsXsi = xmlSearchNsByHref(
        document, root,(xmlChar*)  NS_SCHEMA_INSTANCE);

    //<enabled xsi:nil="true"
    xmlNode  * nodeEn = xmlNewTextChild(
        root,NULL, (xmlChar*) "enabled", (xmlChar*) "");
    if (nodeEn == NULL)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);
    xmlSetNsProp(nodeEn,nsXsi,(xmlChar*) "nil",(xmlChar*) "true");
    //add a new line
    xmlNode * nodeCrLf = xmlNewText((xmlChar*) "\n");
    xmlAddChild(root, nodeCrLf);

    //<userClassPath xsi:nil="true">
    xmlNode  * nodeUs = xmlNewTextChild(
        root,NULL, (xmlChar*) "userClassPath", (xmlChar*) "");
    if (nodeUs == NULL)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);
    xmlSetNsProp(nodeUs,nsXsi,(xmlChar*) "nil",(xmlChar*) "true");
    //add a new line
    nodeCrLf = xmlNewText((xmlChar*) "\n");
    xmlAddChild(root, nodeCrLf);

    //<vmParameters xsi:nil="true">
    xmlNode  * nodeVm = xmlNewTextChild(
        root,NULL, (xmlChar*) "vmParameters", (xmlChar*) "");
    if (nodeVm == NULL)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);
    xmlSetNsProp(nodeVm,nsXsi,(xmlChar*) "nil",(xmlChar*) "true");
    //add a new line
    nodeCrLf = xmlNewText((xmlChar*) "\n");
    xmlAddChild(root, nodeCrLf);

    //<jreLocations xsi:nil="true">
    xmlNode  * nodeJre = xmlNewTextChild(
        root,NULL, (xmlChar*) "jreLocations", (xmlChar*) "");
    if (nodeJre == NULL)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);
    xmlSetNsProp(nodeJre,nsXsi,(xmlChar*) "nil",(xmlChar*) "true");
    //add a new line
    nodeCrLf = xmlNewText((xmlChar*) "\n");
    xmlAddChild(root, nodeCrLf);

    //<javaInfo xsi:nil="true">
    xmlNode  * nodeJava = xmlNewTextChild(
        root,NULL, (xmlChar*) "javaInfo", (xmlChar*) "");
    if (nodeJava == NULL)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);
    xmlSetNsProp(nodeJava,nsXsi,(xmlChar*) "nil",(xmlChar*) "true");
    //add a new line
    nodeCrLf = xmlNewText((xmlChar*) "\n");
    xmlAddChild(root, nodeCrLf);
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
    osl::MutexGuard guard(FwkMutex::get());
    if (arVersions != NULL)
        return arVersions;

    arVersions = new rtl_uString*[vecExcludeVersions.size()];
    int j=0;
    typedef std::vector<rtl::OUString>::const_iterator it;
    for (it i = vecExcludeVersions.begin(); i != vecExcludeVersions.end();
         ++i, ++j)
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

NodeJava::NodeJava(Layer layer):
    m_layer(layer)
{
    //This class reads and write to files which should only be done in
    //application mode
    if (getMode() == JFW_MODE_DIRECT)
        throw FrameworkException(
            JFW_E_DIRECT_MODE,
            "[Java framework] Trying to access settings files in direct mode.");
}


void NodeJava::load()
{
    const rtl::OString sExcMsg("[Java framework] Error in function NodeJava::load"
                             "(elements.cxx).");
    if (SHARED == m_layer)
    {
        //we do not support yet to write into the shared installation

        //check if shared settings exist at all.
        jfw::FileStatus s = checkFileURL(BootParams::getSharedData());
        if (s == FILE_INVALID)
            throw FrameworkException(
                JFW_E_ERROR,
                "[Java framework] Invalid file for shared Java settings.");
        else if (s == FILE_DOES_NOT_EXIST)
            //Writing shared data is not supported yet.
            return;
    }
    else if (USER == m_layer)
    {
        prepareSettingsDocument();
    }
    else
    {
        OSL_FAIL("[Java framework] Unknown enum used.");
    }


    //Read the user elements
    rtl::OString sSettingsPath = getSettingsPath();
    //There must not be a share settings file
    CXmlDocPtr docUser(xmlParseFile(sSettingsPath.getStr()));
    if (docUser == NULL)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);

    xmlNode * cur = xmlDocGetRootElement(docUser);
    if (cur == NULL || cur->children == NULL)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);

    CXmlCharPtr sNil;
    cur = cur->children;
    while (cur != NULL)
    {
        if (xmlStrcmp(cur->name, (xmlChar*) "enabled") == 0)
        {
            //only overwrite share settings if xsi:nil="false"
            sNil = xmlGetNsProp(
                cur, (xmlChar*) "nil", (xmlChar*) NS_SCHEMA_INSTANCE);
            if (sNil == NULL)
                throw FrameworkException(JFW_E_ERROR, sExcMsg);;
            if (xmlStrcmp(sNil, (xmlChar*) "false") == 0)
            {
                CXmlCharPtr sEnabled( xmlNodeListGetString(
                    docUser, cur->children, 1));
                if (xmlStrcmp(sEnabled, (xmlChar*) "true") == 0)
                    m_enabled = boost::optional<sal_Bool>(sal_True);
                else if (xmlStrcmp(sEnabled, (xmlChar*) "false") == 0)
                    m_enabled = boost::optional<sal_Bool>(sal_False);
            }
        }
        else if (xmlStrcmp(cur->name, (xmlChar*) "userClassPath") == 0)
        {
            sNil = xmlGetNsProp(
                cur, (xmlChar*) "nil", (xmlChar*) NS_SCHEMA_INSTANCE);
            if (sNil == NULL)
                throw FrameworkException(JFW_E_ERROR, sExcMsg);
            if (xmlStrcmp(sNil, (xmlChar*) "false") == 0)
            {
                CXmlCharPtr sUser(xmlNodeListGetString(
                    docUser, cur->children, 1));
                m_userClassPath = boost::optional<rtl::OUString>(rtl::OUString(sUser));
            }
        }
        else if (xmlStrcmp(cur->name, (xmlChar*) "javaInfo") == 0)
        {
            sNil = xmlGetNsProp(
                cur, (xmlChar*) "nil", (xmlChar*) NS_SCHEMA_INSTANCE);
            if (sNil == NULL)
                throw FrameworkException(JFW_E_ERROR, sExcMsg);

            if (xmlStrcmp(sNil, (xmlChar*) "false") == 0)
            {
                if (! m_javaInfo)
                    m_javaInfo = boost::optional<CNodeJavaInfo>(CNodeJavaInfo());
                m_javaInfo->loadFromNode(docUser, cur);
            }
        }
        else if (xmlStrcmp(cur->name, (xmlChar*) "vmParameters") == 0)
        {
            sNil = xmlGetNsProp(
                cur, (xmlChar*) "nil", (xmlChar*) NS_SCHEMA_INSTANCE);
            if (sNil == NULL)
                throw FrameworkException(JFW_E_ERROR, sExcMsg);
            if (xmlStrcmp(sNil, (xmlChar*) "false") == 0)
            {
                if ( ! m_vmParameters)
                    m_vmParameters = boost::optional<std::vector<rtl::OUString> >(
                        std::vector<rtl::OUString> ());

                xmlNode * pOpt = cur->children;
                while (pOpt != NULL)
                {
                    if (xmlStrcmp(pOpt->name, (xmlChar*) "param") == 0)
                    {
                        CXmlCharPtr sOpt;
                        sOpt = xmlNodeListGetString(
                            docUser, pOpt->children, 1);
                        m_vmParameters->push_back(sOpt);
                    }
                    pOpt = pOpt->next;
                }
            }
        }
        else if (xmlStrcmp(cur->name, (xmlChar*) "jreLocations") == 0)
        {
            sNil = xmlGetNsProp(
                cur, (xmlChar*) "nil", (xmlChar*) NS_SCHEMA_INSTANCE);
            if (sNil == NULL)
                throw FrameworkException(JFW_E_ERROR, sExcMsg);
            if (xmlStrcmp(sNil, (xmlChar*) "false") == 0)
            {
                if (! m_JRELocations)
                    m_JRELocations = boost::optional<std::vector<rtl::OUString> >(
                        std::vector<rtl::OUString>());

                xmlNode * pLoc = cur->children;
                while (pLoc != NULL)
                {
                    if (xmlStrcmp(pLoc->name, (xmlChar*) "location") == 0)
                    {
                        CXmlCharPtr sLoc;
                        sLoc = xmlNodeListGetString(
                            docUser, pLoc->children, 1);
                        m_JRELocations->push_back(sLoc);
                    }
                    pLoc = pLoc->next;
                }
            }
        }
        cur = cur->next;
    }
}

::rtl::OString NodeJava::getSettingsPath() const
{
    ::rtl::OString ret;
    switch (m_layer)
    {
    case USER: ret = getUserSettingsPath(); break;
    case SHARED: ret = getSharedSettingsPath(); break;
    default:
        OSL_FAIL("[Java framework] NodeJava::getSettingsPath()");
    }
    return ret;
}

::rtl::OUString NodeJava::getSettingsURL() const
{
    ::rtl::OUString ret;
    switch (m_layer)
    {
    case USER: ret = BootParams::getUserData(); break;
    case SHARED: ret = BootParams::getSharedData(); break;
    default:
        OSL_FAIL("[Java framework] NodeJava::getSettingsURL()");
    }
    return ret;
}

void NodeJava::prepareSettingsDocument() const
{
    rtl::OString sExcMsg(
        "[Java framework] Error in function prepareSettingsDocument"
        " (elements.cxx).");
    createSettingsDocument();
    rtl::OString sSettings = getSettingsPath();
    CXmlDocPtr doc(xmlParseFile(sSettings.getStr()));
    if (!doc)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);

    bool bNeedsSave = false;
    createSettingsStructure(doc, & bNeedsSave);
    if (bNeedsSave)
    {
        if (xmlSaveFormatFileEnc(
                sSettings.getStr(), doc,"UTF-8", 1) == -1)
            throw FrameworkException(JFW_E_ERROR, sExcMsg);
    }
}

void NodeJava::write() const
{
    rtl::OString sExcMsg("[Java framework] Error in function NodeJava::writeSettings "
                         "(elements.cxx).");
    CXmlDocPtr docUser;
    CXPathContextPtr contextUser;
    CXPathObjectPtr pathObj;

    prepareSettingsDocument();

    //Read the user elements
    rtl::OString sSettingsPath = getSettingsPath();
    docUser = xmlParseFile(sSettingsPath.getStr());
    if (docUser == NULL)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);
    contextUser = xmlXPathNewContext(docUser);
    if (xmlXPathRegisterNs(contextUser, (xmlChar*) "jf",
        (xmlChar*) NS_JAVA_FRAMEWORK) == -1)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);

    xmlNode * root = xmlDocGetRootElement(docUser);
    //Get xsi:nil namespace
    xmlNs* nsXsi = xmlSearchNsByHref(docUser,
                             root,
                             (xmlChar*)  NS_SCHEMA_INSTANCE);

    //set the <enabled> element
    //The element must exist
    if (m_enabled)
    {
        rtl::OString sExpression= rtl::OString(
            "/jf:java/jf:enabled");
        pathObj = xmlXPathEvalExpression((xmlChar*) sExpression.getStr(),
                                         contextUser);
        if ( ! pathObj || xmlXPathNodeSetIsEmpty(pathObj->nodesetval))
            throw FrameworkException(JFW_E_ERROR, sExcMsg);

        xmlNode * nodeEnabled = pathObj->nodesetval->nodeTab[0];
        xmlSetNsProp(nodeEnabled,
                     nsXsi,
                     (xmlChar*) "nil",
                     (xmlChar*) "false");

        if (m_enabled == boost::optional<sal_Bool>(sal_True))
            xmlNodeSetContent(nodeEnabled,(xmlChar*) "true");
        else
            xmlNodeSetContent(nodeEnabled,(xmlChar*) "false");
    }

    //set the <userClassPath> element
    //The element must exist
    if (m_userClassPath)
    {
        rtl::OString sExpression= rtl::OString(
            "/jf:java/jf:userClassPath");
        pathObj = xmlXPathEvalExpression((xmlChar*) sExpression.getStr(),
                                         contextUser);
        if ( ! pathObj || xmlXPathNodeSetIsEmpty(pathObj->nodesetval))
            throw FrameworkException(JFW_E_ERROR, sExcMsg);

        xmlNode * nodeEnabled = pathObj->nodesetval->nodeTab[0];
        xmlSetNsProp(nodeEnabled, nsXsi, (xmlChar*) "nil",(xmlChar*) "false");
        xmlNodeSetContent(nodeEnabled,(xmlChar*) CXmlCharPtr(*m_userClassPath));
    }

    //set <javaInfo> element
    if (m_javaInfo)
    {
        rtl::OString sExpression= rtl::OString(
            "/jf:java/jf:javaInfo");
        pathObj = xmlXPathEvalExpression((xmlChar*) sExpression.getStr(),
                                                contextUser);
        if ( ! pathObj || xmlXPathNodeSetIsEmpty(pathObj->nodesetval))
            throw FrameworkException(JFW_E_ERROR, sExcMsg);
        m_javaInfo->writeToNode(
            docUser, pathObj->nodesetval->nodeTab[0]);
    }

    //set <vmParameters> element
    if (m_vmParameters)
    {
        rtl::OString sExpression= rtl::OString(
            "/jf:java/jf:vmParameters");
        pathObj = xmlXPathEvalExpression((xmlChar*) sExpression.getStr(),
                                         contextUser);
        if ( ! pathObj || xmlXPathNodeSetIsEmpty(pathObj->nodesetval))
            throw FrameworkException(JFW_E_ERROR, sExcMsg);
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
        if (m_vmParameters->size() > 0)
        {
            xmlNode * nodeCrLf = xmlNewText((xmlChar*) "\n");
            xmlAddChild(vmParameters, nodeCrLf);
        }

        typedef std::vector<rtl::OUString>::const_iterator cit;
        for (cit i = m_vmParameters->begin(); i != m_vmParameters->end(); ++i)
        {
            xmlNewTextChild(vmParameters, NULL, (xmlChar*) "param",
                            CXmlCharPtr(*i));
            //add a new line
            xmlNode * nodeCrLf = xmlNewText((xmlChar*) "\n");
            xmlAddChild(vmParameters, nodeCrLf);
        }
    }

    //set <jreLocations> element
    if (m_JRELocations)
    {
        rtl::OString sExpression= rtl::OString(
            "/jf:java/jf:jreLocations");
        pathObj = xmlXPathEvalExpression((xmlChar*) sExpression.getStr(),
                                         contextUser);
        if ( ! pathObj || xmlXPathNodeSetIsEmpty(pathObj->nodesetval))
            throw FrameworkException(JFW_E_ERROR, sExcMsg);
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
        if (m_JRELocations->size() > 0)
        {
            xmlNode * nodeCrLf = xmlNewText((xmlChar*) "\n");
            xmlAddChild(jreLocationsNode, nodeCrLf);
        }

        typedef std::vector<rtl::OUString>::const_iterator cit;
        for (cit i = m_JRELocations->begin(); i != m_JRELocations->end(); ++i)
        {
            xmlNewTextChild(jreLocationsNode, NULL, (xmlChar*) "location",
                            CXmlCharPtr(*i));
            //add a new line
            xmlNode * nodeCrLf = xmlNewText((xmlChar*) "\n");
            xmlAddChild(jreLocationsNode, nodeCrLf);
        }
    }

    if (xmlSaveFormatFile(sSettingsPath.getStr(), docUser, 1) == -1)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);
}

void NodeJava::setEnabled(sal_Bool bEnabled)
{
    m_enabled =  boost::optional<sal_Bool>(bEnabled);
}


void NodeJava::setUserClassPath(const rtl::OUString & sClassPath)
{
    m_userClassPath = boost::optional<rtl::OUString>(sClassPath);
}

void NodeJava::setJavaInfo(const JavaInfo * pInfo, bool bAutoSelect)
{
    if (!m_javaInfo)
        m_javaInfo = boost::optional<CNodeJavaInfo>(CNodeJavaInfo());
    m_javaInfo->bAutoSelect = bAutoSelect;
    m_javaInfo->bNil = false;

    if (pInfo != NULL)
    {
        m_javaInfo->m_bEmptyNode = false;
        m_javaInfo->sVendor = pInfo->sVendor;
        m_javaInfo->sLocation = pInfo->sLocation;
        m_javaInfo->sVersion = pInfo->sVersion;
        m_javaInfo->nFeatures = pInfo->nFeatures;
        m_javaInfo->nRequirements = pInfo->nRequirements;
        m_javaInfo->arVendorData = pInfo->arVendorData;
    }
    else
    {
        m_javaInfo->m_bEmptyNode = true;
        rtl::OUString sEmpty;
        m_javaInfo->sVendor = sEmpty;
        m_javaInfo->sLocation = sEmpty;
        m_javaInfo->sVersion = sEmpty;
        m_javaInfo->nFeatures = 0;
        m_javaInfo->nRequirements = 0;
        m_javaInfo->arVendorData = rtl::ByteSequence();
    }
}

void NodeJava::setVmParameters(rtl_uString * * arOptions, sal_Int32 size)
{
    OSL_ASSERT( !(arOptions == 0 && size != 0));
    if ( ! m_vmParameters)
        m_vmParameters = boost::optional<std::vector<rtl::OUString> >(
            std::vector<rtl::OUString>());
    m_vmParameters->clear();
    if (arOptions != NULL)
    {
        for (int i  = 0; i < size; i++)
        {
            const rtl::OUString sOption(static_cast<rtl_uString*>(arOptions[i]));
            m_vmParameters->push_back(sOption);
        }
    }
}

void NodeJava::setJRELocations(rtl_uString  * * arLocations, sal_Int32 size)
{
    OSL_ASSERT( !(arLocations == 0 && size != 0));
    if (! m_JRELocations)
        m_JRELocations = boost::optional<std::vector<rtl::OUString> > (
            std::vector<rtl::OUString>());
    m_JRELocations->clear();
    if (arLocations != NULL)
    {
        for (int i  = 0; i < size; i++)
        {
            const rtl::OUString & sLocation = static_cast<rtl_uString*>(arLocations[i]);

            //only add the path if not already present
            std::vector<rtl::OUString>::const_iterator it =
                std::find(m_JRELocations->begin(), m_JRELocations->end(),
                          sLocation);
            if (it == m_JRELocations->end())
                m_JRELocations->push_back(sLocation);
        }
    }
}

void NodeJava::addJRELocation(rtl_uString * sLocation)
{
    OSL_ASSERT( sLocation);
    if (!m_JRELocations)
        m_JRELocations = boost::optional<std::vector<rtl::OUString> >(
            std::vector<rtl::OUString> ());
     //only add the path if not already present
    std::vector<rtl::OUString>::const_iterator it =
        std::find(m_JRELocations->begin(), m_JRELocations->end(),
                  rtl::OUString(sLocation));
    if (it == m_JRELocations->end())
        m_JRELocations->push_back(rtl::OUString(sLocation));
}

const boost::optional<sal_Bool> & NodeJava::getEnabled() const
{
    return m_enabled;
}

const boost::optional<std::vector<rtl::OUString> >&
NodeJava::getJRELocations() const
{
    return m_JRELocations;
}

const boost::optional<rtl::OUString> & NodeJava::getUserClassPath() const
{
    return m_userClassPath;
}

const boost::optional<std::vector<rtl::OUString> > & NodeJava::getVmParameters() const
{
    return m_vmParameters;
}

const boost::optional<CNodeJavaInfo> & NodeJava::getJavaInfo() const
{
    return m_javaInfo;
}

jfw::FileStatus NodeJava::checkSettingsFileStatus() const
{
    jfw::FileStatus ret = FILE_DOES_NOT_EXIST;

    const rtl::OUString sURL = getSettingsURL();
    //check the file time
    ::osl::DirectoryItem item;
    File::RC rc = ::osl::DirectoryItem::get(sURL, item);
    if (File::E_None == rc)
    {
        ::osl::FileStatus stat(osl_FileStatus_Mask_Validate);
        File::RC rc_stat = item.getFileStatus(stat);
        if (File::E_None == rc_stat)
        {
            ret = FILE_OK;
        }
        else if (File::E_NOENT == rc_stat)
        {
            ret = FILE_DOES_NOT_EXIST;
        }
        else
        {
            ret = FILE_INVALID;
        }
    }
    else if(File::E_NOENT == rc)
    {
        ret = FILE_DOES_NOT_EXIST;
    }
    else
    {
        ret = FILE_INVALID;
    }
    return ret;
}

void NodeJava::createSettingsDocument() const
{
    const rtl::OUString sURL = getSettingsURL();
    //make sure there is a user directory
    rtl::OString sExcMsg("[Java framework] Error in function createSettingsDocument "
                         "(elements.cxx).");
    // check if javasettings.xml already exist
    if (FILE_OK == checkSettingsFileStatus())
        return;

    //make sure that the directories are created in case they do not exist
    FileBase::RC rcFile = Directory::createPath(getDirFromFile(sURL));
    if (rcFile != FileBase::E_EXIST && rcFile != FileBase::E_None)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);

    //javasettings.xml does not exist yet
    CXmlDocPtr doc(xmlNewDoc((xmlChar *)"1.0"));
    if (! doc)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);
    //Create a comment
    xmlNewDocComment(
        doc, (xmlChar *) "This is a generated file. Do not alter this file!");

    //Create the root element and name spaces
    xmlNodePtr root =   xmlNewDocNode(
        doc, NULL, (xmlChar *) "java", (xmlChar *) "\n");

    if (root == NULL)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);

    if (xmlNewNs(root, (xmlChar *) NS_JAVA_FRAMEWORK,NULL) == NULL)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);
    if (xmlNewNs(root,(xmlChar*) NS_SCHEMA_INSTANCE,(xmlChar*)"xsi") == NULL)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);
    xmlDocSetRootElement(doc,  root);

    //Create a comment
    xmlNodePtr com = xmlNewComment(
        (xmlChar *) "This is a generated file. Do not alter this file!");
    if (com == NULL)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);

    if (xmlAddPrevSibling(root, com) == NULL)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);

    const rtl::OString path = getSettingsPath();
    if (xmlSaveFormatFileEnc(path.getStr(), doc,"UTF-8", 1) == -1)
         throw FrameworkException(JFW_E_ERROR, sExcMsg);
}

//=====================================================================
CNodeJavaInfo::CNodeJavaInfo() :
    m_bEmptyNode(false), bNil(true), bAutoSelect(true),
    nFeatures(0), nRequirements(0)
{
}

CNodeJavaInfo::~CNodeJavaInfo()
{
}

void CNodeJavaInfo::loadFromNode(xmlDoc * pDoc, xmlNode * pJavaInfo)
{
    rtl::OString sExcMsg("[Java framework] Error in function NodeJavaInfo::loadFromNode "
                         "(elements.cxx).");

    OSL_ASSERT(pJavaInfo && pDoc);
    if (pJavaInfo->children == NULL)
        return;
    //Get the xsi:nil attribute;
    CXmlCharPtr sNil;
    sNil = xmlGetNsProp(
        pJavaInfo, (xmlChar*) "nil", (xmlChar*) NS_SCHEMA_INSTANCE);
    if ( ! sNil)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);

    if (xmlStrcmp(sNil, (xmlChar*) "true") == 0)
        bNil = true;
    else if (xmlStrcmp(sNil, (xmlChar*) "false") == 0)
        bNil = false;
    else
        throw FrameworkException(JFW_E_ERROR, sExcMsg);
    if (bNil == true)
        return;

    //Get javaInfo@manuallySelected attribute
    CXmlCharPtr sAutoSelect;
    sAutoSelect = xmlGetProp(
        pJavaInfo, (xmlChar*) "autoSelect");
    if ( ! sAutoSelect)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);

    if (xmlStrcmp(sAutoSelect, (xmlChar*) "true") == 0)
        bAutoSelect = true;
    else if (xmlStrcmp(sAutoSelect, (xmlChar*) "false") == 0)
        bAutoSelect = false;
    else
        throw FrameworkException(JFW_E_ERROR, sExcMsg);

    xmlNode * cur = pJavaInfo->children;

    while (cur != NULL)
    {
        if (xmlStrcmp(cur->name, (xmlChar*) "vendor") == 0)
        {
            CXmlCharPtr xmlVendor;
            xmlVendor = xmlNodeListGetString(
                pDoc, cur->children, 1);
            if (! xmlVendor)
                return;
            sVendor = xmlVendor;
        }
        else if (xmlStrcmp(cur->name, (xmlChar*) "location") == 0)
        {
            CXmlCharPtr xmlLocation;
            xmlLocation = xmlNodeListGetString(
                pDoc, cur->children, 1);
            sLocation = xmlLocation;
        }
        else if (xmlStrcmp(cur->name, (xmlChar*) "version") == 0)
        {
            CXmlCharPtr xmlVersion;
            xmlVersion = xmlNodeListGetString(
                pDoc, cur->children, 1);
            sVersion = xmlVersion;
        }
        else if (xmlStrcmp(cur->name, (xmlChar*) "features")== 0)
        {
            CXmlCharPtr xmlFeatures;
            xmlFeatures = xmlNodeListGetString(
                    pDoc, cur->children, 1);
            rtl::OUString sFeatures = xmlFeatures;
            nFeatures = sFeatures.toInt64(16);
        }
        else if (xmlStrcmp(cur->name, (xmlChar*) "requirements") == 0)
        {
            CXmlCharPtr xmlRequire;
            xmlRequire = xmlNodeListGetString(
                pDoc, cur->children, 1);
            rtl::OUString sRequire = xmlRequire;
            nRequirements = sRequire.toInt64(16);
#ifdef MACOSX
            //javaldx is not used anymore in the mac build. In case the Java
            //corresponding to the saved settings does not exist anymore the
            //javavm services will look for an existing Java after creation of
            //the JVM failed. See stoc/source/javavm/javavm.cxx. Only if
            //nRequirements does not have the flag JFW_REQUIRE_NEEDRESTART the
            //jvm of the new selected JRE will be started. Old settings (before
            //OOo 3.3) still contain the flag which can be safely ignored.
            nRequirements &= ~JFW_REQUIRE_NEEDRESTART;
#endif
        }
        else if (xmlStrcmp(cur->name, (xmlChar*) "vendorData") == 0)
        {
            CXmlCharPtr xmlData;
            xmlData = xmlNodeListGetString(
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

    if (sVendor.isEmpty())
        m_bEmptyNode = true;
    //Get the javainfo attributes
    CXmlCharPtr sVendorUpdate;
    sVendorUpdate = xmlGetProp(pJavaInfo,
                               (xmlChar*) "vendorUpdate");
    if ( ! sVendorUpdate)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);
    sAttrVendorUpdate = sVendorUpdate;
}


void CNodeJavaInfo::writeToNode(xmlDoc* pDoc,
                                xmlNode* pJavaInfoNode) const

{
    OSL_ASSERT(pJavaInfoNode && pDoc);
    //write the attribute vendorSettings

    //javaInfo@vendorUpdate
    //creates the attribute if necessary
    rtl::OString sUpdated = getElementUpdated();

    xmlSetProp(pJavaInfoNode, (xmlChar*)"vendorUpdate",
               (xmlChar*) sUpdated.getStr());

    //javaInfo@autoSelect
    xmlSetProp(pJavaInfoNode, (xmlChar*)"autoSelect",
               (xmlChar*) (bAutoSelect == true ? "true" : "false"));

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
        return;

    //add a new line after <javaInfo>
    xmlNode * nodeCrLf = xmlNewText((xmlChar*) "\n");
    xmlAddChild(pJavaInfoNode, nodeCrLf);

    //Create the vendor element
    xmlNewTextChild(pJavaInfoNode, NULL, (xmlChar*) "vendor",
                    CXmlCharPtr(sVendor));
    //add a new line for better readability
    nodeCrLf = xmlNewText((xmlChar*) "\n");
    xmlAddChild(pJavaInfoNode, nodeCrLf);

    //Create the location element
    xmlNewTextChild(pJavaInfoNode, NULL, (xmlChar*) "location",
                    CXmlCharPtr(sLocation));
    //add a new line for better readability
    nodeCrLf = xmlNewText((xmlChar*) "\n");
    xmlAddChild(pJavaInfoNode, nodeCrLf);

    //Create the version element
    xmlNewTextChild(pJavaInfoNode, NULL, (xmlChar*) "version",
                    CXmlCharPtr(sVersion));
    //add a new line for better readability
    nodeCrLf = xmlNewText((xmlChar*) "\n");
    xmlAddChild(pJavaInfoNode, nodeCrLf);

    //Create the features element
    rtl::OUString sFeatures = rtl::OUString::valueOf(
        (sal_Int64)nFeatures, 16);
    xmlNewTextChild(pJavaInfoNode, NULL, (xmlChar*) "features",
                    CXmlCharPtr(sFeatures));
    //add a new line for better readability
    nodeCrLf = xmlNewText((xmlChar*) "\n");
    xmlAddChild(pJavaInfoNode, nodeCrLf);


    //Create the requirements element
    rtl::OUString sRequirements = rtl::OUString::valueOf(
        (sal_Int64) nRequirements, 16);
    xmlNewTextChild(pJavaInfoNode, NULL, (xmlChar*) "requirements",
                    CXmlCharPtr(sRequirements));
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
}

JavaInfo * CNodeJavaInfo::makeJavaInfo() const
{
    if (bNil == true || m_bEmptyNode == true)
        return NULL;
    JavaInfo * pInfo = (JavaInfo*) rtl_allocateMemory(sizeof(JavaInfo));
    if (pInfo == NULL)
        return NULL;
    memset(pInfo, 0, sizeof(JavaInfo));
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

//================================================================================
MergedSettings::MergedSettings():
    m_bEnabled(false),
    m_sClassPath(),
    m_vmParams(),
    m_JRELocations(),
    m_javaInfo()
{
    NodeJava settings(NodeJava::USER);
    settings.load();
    NodeJava sharedSettings(NodeJava::SHARED);
    sharedSettings.load();
    merge(sharedSettings, settings);
}

MergedSettings::~MergedSettings()
{
}

void MergedSettings::merge(const NodeJava & share, const NodeJava & user)
{
    if (user.getEnabled())
        m_bEnabled = * user.getEnabled();
    else if (share.getEnabled())
        m_bEnabled = * share.getEnabled();
    else
        m_bEnabled = sal_True;

    if (user.getUserClassPath())
        m_sClassPath = * user.getUserClassPath();
    else if (share.getUserClassPath())
        m_sClassPath = * share.getUserClassPath();

    if (user.getJavaInfo())
        m_javaInfo = * user.getJavaInfo();
    else if (share.getJavaInfo())
        m_javaInfo = * share.getJavaInfo();

    if (user.getVmParameters())
        m_vmParams = * user.getVmParameters();
    else if (share.getVmParameters())
         m_vmParams = * share.getVmParameters();

    if (user.getJRELocations())
        m_JRELocations = * user.getJRELocations();
    else if (share.getJRELocations())
        m_JRELocations = * share.getJRELocations();
}

bool MergedSettings::getEnabled() const
{
    return m_bEnabled;
}
const rtl::OUString&  MergedSettings::getUserClassPath() const
{
    return m_sClassPath;
}

::std::vector< ::rtl::OString> MergedSettings::getVmParametersUtf8() const
{
    ::std::vector< ::rtl::OString> ret;
    typedef ::std::vector< ::rtl::OUString>::const_iterator cit;
    for (cit i = m_vmParams.begin(); i != m_vmParams.end(); ++i)
    {
        ret.push_back( ::rtl::OUStringToOString(*i, RTL_TEXTENCODING_UTF8));
    }
    return ret;
}

const ::rtl::OString  & MergedSettings::getJavaInfoAttrVendorUpdate() const
{
    return m_javaInfo.sAttrVendorUpdate;
}


JavaInfo * MergedSettings::createJavaInfo() const
{
    return m_javaInfo.makeJavaInfo();
}
#ifdef WNT
bool MergedSettings::getJavaInfoAttrAutoSelect() const
{
    return m_javaInfo.bAutoSelect;
}
#endif
void MergedSettings::getVmParametersArray(
    rtl_uString *** parParams, sal_Int32 * size) const
{
    osl::MutexGuard guard(FwkMutex::get());
    OSL_ASSERT(parParams != NULL && size != NULL);

    *parParams = (rtl_uString **)
        rtl_allocateMemory(sizeof(rtl_uString*) * m_vmParams.size());
    if (*parParams == NULL)
        return;

    int j=0;
    typedef std::vector<rtl::OUString>::const_iterator it;
    for (it i = m_vmParams.begin(); i != m_vmParams.end();
         ++i, ++j)
    {
        (*parParams)[j] = i->pData;
        rtl_uString_acquire(i->pData);
    }
    *size = m_vmParams.size();
}

void MergedSettings::getJRELocations(
    rtl_uString *** parLocations, sal_Int32 * size) const
{
    osl::MutexGuard guard(FwkMutex::get());
    OSL_ASSERT(parLocations != NULL && size != NULL);

    *parLocations = (rtl_uString **)
        rtl_allocateMemory(sizeof(rtl_uString*) * m_JRELocations.size());
    if (*parLocations == NULL)
        return;

    int j=0;
    typedef std::vector<rtl::OUString>::const_iterator it;
    for (it i = m_JRELocations.begin(); i != m_JRELocations.end();
         ++i, ++j)
    {
        (*parLocations)[j] = i->pData;
        rtl_uString_acquire(i->pData);
    }
    *size = m_JRELocations.size();
}
const std::vector<rtl::OUString> & MergedSettings::getJRELocations() const
{
    return m_JRELocations;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
