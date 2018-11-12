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

#include <sal/config.h>
#include <sal/log.hxx>

#include <cassert>
#include <memory>

#include <elements.hxx>
#include <osl/mutex.hxx>
#include <osl/file.hxx>
#include <fwkutil.hxx>
#include "fwkbase.hxx"
#include "framework.hxx"
#include "libxmlutil.hxx"
#include <osl/thread.hxx>
#include <algorithm>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <rtl/bootstrap.hxx>
#include <boost/optional.hpp>
#include <string.h>

// For backwards compatibility, the nFeatures and nRequirements flag words are
// read/written as potentially signed hexadecimal numbers (though that has no
// practical relevance given that each has only one flag with value 0x01
// defined).

using namespace osl;
namespace jfw
{

static OString getElement(OString const & docPath,
                        xmlChar const * pathExpression)
{
    //Prepare the xml document and context
    OSL_ASSERT(!docPath.isEmpty());
    jfw::CXmlDocPtr doc(xmlParseFile(docPath.getStr()));
    if (doc == nullptr)
        throw FrameworkException(
            JFW_E_ERROR,
            "[Java framework] Error in function getElement (elements.cxx)");

    jfw::CXPathContextPtr context(xmlXPathNewContext(doc));
    if (xmlXPathRegisterNs(context, reinterpret_cast<xmlChar const *>("jf"),
        reinterpret_cast<xmlChar const *>(NS_JAVA_FRAMEWORK)) == -1)
        throw FrameworkException(
            JFW_E_ERROR,
            "[Java framework] Error in function getElement (elements.cxx)");

    CXPathObjectPtr pathObj;
    pathObj = xmlXPathEvalExpression(pathExpression, context);
    OString sValue;
    if (xmlXPathNodeSetIsEmpty(pathObj->nodesetval))
    {
        throw FrameworkException(
            JFW_E_ERROR,
            "[Java framework] Error in function getElement (elements.cxx)");
    }
    sValue = reinterpret_cast<sal_Char*>(pathObj->nodesetval->nodeTab[0]->content);
    return sValue;
}

OString getElementUpdated()
{
    return getElement(jfw::getVendorSettingsPath(),
                      reinterpret_cast<xmlChar const *>("/jf:javaSelection/jf:updated/text()"));
}

void createSettingsStructure(xmlDoc * document, bool * bNeedsSave)
{
    OString sExcMsg("[Java framework] Error in function createSettingsStructure "
                         "(elements.cxx).");
    xmlNode * root = xmlDocGetRootElement(document);
    if (root == nullptr)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);
    bool bFound = false;
    xmlNode * cur = root->children;
    while (cur != nullptr)
    {
        if (xmlStrcmp(cur->name, reinterpret_cast<xmlChar const *>("enabled")) == 0)
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
        document, root, reinterpret_cast<xmlChar const *>(NS_SCHEMA_INSTANCE));

    //<enabled xsi:nil="true"
    xmlNode  * nodeEn = xmlNewTextChild(
        root, nullptr, reinterpret_cast<xmlChar const *>("enabled"), reinterpret_cast<xmlChar const *>(""));
    if (nodeEn == nullptr)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);
    xmlSetNsProp(nodeEn, nsXsi, reinterpret_cast<xmlChar const *>("nil"), reinterpret_cast<xmlChar const *>("true"));
    //add a new line
    xmlNode * nodeCrLf = xmlNewText(reinterpret_cast<xmlChar const *>("\n"));
    xmlAddChild(root, nodeCrLf);

    //<userClassPath xsi:nil="true">
    xmlNode  * nodeUs = xmlNewTextChild(
        root, nullptr, reinterpret_cast<xmlChar const *>("userClassPath"), reinterpret_cast<xmlChar const *>(""));
    if (nodeUs == nullptr)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);
    xmlSetNsProp(nodeUs, nsXsi, reinterpret_cast<xmlChar const *>("nil"), reinterpret_cast<xmlChar const *>("true"));
    //add a new line
    nodeCrLf = xmlNewText(reinterpret_cast<xmlChar const *>("\n"));
    xmlAddChild(root, nodeCrLf);

    //<vmParameters xsi:nil="true">
    xmlNode  * nodeVm = xmlNewTextChild(
        root, nullptr, reinterpret_cast<xmlChar const *>("vmParameters"), reinterpret_cast<xmlChar const *>(""));
    if (nodeVm == nullptr)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);
    xmlSetNsProp(nodeVm, nsXsi, reinterpret_cast<xmlChar const *>("nil"), reinterpret_cast<xmlChar const *>("true"));
    //add a new line
    nodeCrLf = xmlNewText(reinterpret_cast<xmlChar const *>("\n"));
    xmlAddChild(root, nodeCrLf);

    //<jreLocations xsi:nil="true">
    xmlNode  * nodeJre = xmlNewTextChild(
        root, nullptr, reinterpret_cast<xmlChar const *>("jreLocations"), reinterpret_cast<xmlChar const *>(""));
    if (nodeJre == nullptr)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);
    xmlSetNsProp(nodeJre, nsXsi, reinterpret_cast<xmlChar const *>("nil"), reinterpret_cast<xmlChar const *>("true"));
    //add a new line
    nodeCrLf = xmlNewText(reinterpret_cast<xmlChar const *>("\n"));
    xmlAddChild(root, nodeCrLf);

    //<javaInfo xsi:nil="true">
    xmlNode  * nodeJava = xmlNewTextChild(
        root, nullptr, reinterpret_cast<xmlChar const *>("javaInfo"), reinterpret_cast<xmlChar const *>(""));
    if (nodeJava == nullptr)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);
    xmlSetNsProp(nodeJava, nsXsi, reinterpret_cast<xmlChar const *>("nil"), reinterpret_cast<xmlChar const *>("true"));
    //add a new line
    nodeCrLf = xmlNewText(reinterpret_cast<xmlChar const *>("\n"));
    xmlAddChild(root, nodeCrLf);
}

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
    const OString sExcMsg("[Java framework] Error in function NodeJava::load"
                             "(elements.cxx).");
    if (SHARED == m_layer)
    {
        //we do not support yet to write into the shared installation

        //check if shared settings exist at all.
        OUString sURL(BootParams::getSharedData());
        jfw::FileStatus s = sURL.isEmpty()
            ? FILE_DOES_NOT_EXIST : checkFileURL(sURL);
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
        if (!prepareSettingsDocument())
        {
            SAL_INFO("jfw.level1", "no path to load user settings document from");
            return;
        }
    }
    else
    {
        OSL_FAIL("[Java framework] Unknown enum used.");
    }


    //Read the user elements
    OString sSettingsPath = getSettingsPath();
    //There must not be a share settings file
    CXmlDocPtr docUser(xmlParseFile(sSettingsPath.getStr()));
    if (docUser == nullptr)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);

    xmlNode * cur = xmlDocGetRootElement(docUser);
    if (cur == nullptr || cur->children == nullptr)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);

    CXmlCharPtr sNil;
    cur = cur->children;
    while (cur != nullptr)
    {
        if (xmlStrcmp(cur->name, reinterpret_cast<xmlChar const *>("enabled")) == 0)
        {
            //only overwrite share settings if xsi:nil="false"
            sNil = xmlGetNsProp(
                cur, reinterpret_cast<xmlChar const *>("nil"), reinterpret_cast<xmlChar const *>(NS_SCHEMA_INSTANCE));
            if (sNil == nullptr)
                throw FrameworkException(JFW_E_ERROR, sExcMsg);
            if (xmlStrcmp(sNil, reinterpret_cast<xmlChar const *>("false")) == 0)
            {
                CXmlCharPtr sEnabled( xmlNodeListGetString(
                    docUser, cur->children, 1));
                if (xmlStrcmp(sEnabled, reinterpret_cast<xmlChar const *>("true")) == 0)
                    m_enabled = boost::optional<sal_Bool>(true);
                else if (xmlStrcmp(sEnabled, reinterpret_cast<xmlChar const *>("false")) == 0)
                    m_enabled = boost::optional<sal_Bool>(false);
            }
        }
        else if (xmlStrcmp(cur->name, reinterpret_cast<xmlChar const *>("userClassPath")) == 0)
        {
            sNil = xmlGetNsProp(
                cur, reinterpret_cast<xmlChar const *>("nil"), reinterpret_cast<xmlChar const *>(NS_SCHEMA_INSTANCE));
            if (sNil == nullptr)
                throw FrameworkException(JFW_E_ERROR, sExcMsg);
            if (xmlStrcmp(sNil, reinterpret_cast<xmlChar const *>("false")) == 0)
            {
                CXmlCharPtr sUser(xmlNodeListGetString(
                    docUser, cur->children, 1));
                m_userClassPath = boost::optional<OUString>(OUString(sUser));
            }
        }
        else if (xmlStrcmp(cur->name, reinterpret_cast<xmlChar const *>("javaInfo")) == 0)
        {
            sNil = xmlGetNsProp(
                cur, reinterpret_cast<xmlChar const *>("nil"), reinterpret_cast<xmlChar const *>(NS_SCHEMA_INSTANCE));
            if (sNil == nullptr)
                throw FrameworkException(JFW_E_ERROR, sExcMsg);

            if (xmlStrcmp(sNil, reinterpret_cast<xmlChar const *>("false")) == 0)
            {
                if (! m_javaInfo)
                    m_javaInfo = boost::optional<CNodeJavaInfo>(CNodeJavaInfo());
                m_javaInfo->loadFromNode(docUser, cur);
            }
        }
        else if (xmlStrcmp(cur->name, reinterpret_cast<xmlChar const *>("vmParameters")) == 0)
        {
            sNil = xmlGetNsProp(
                cur, reinterpret_cast<xmlChar const *>("nil"), reinterpret_cast<xmlChar const *>(NS_SCHEMA_INSTANCE));
            if (sNil == nullptr)
                throw FrameworkException(JFW_E_ERROR, sExcMsg);
            if (xmlStrcmp(sNil, reinterpret_cast<xmlChar const *>("false")) == 0)
            {
                if ( ! m_vmParameters)
                    m_vmParameters = boost::optional<std::vector<OUString> >(
                        std::vector<OUString> ());

                xmlNode * pOpt = cur->children;
                while (pOpt != nullptr)
                {
                    if (xmlStrcmp(pOpt->name, reinterpret_cast<xmlChar const *>("param")) == 0)
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
        else if (xmlStrcmp(cur->name, reinterpret_cast<xmlChar const *>("jreLocations")) == 0)
        {
            sNil = xmlGetNsProp(
                cur, reinterpret_cast<xmlChar const *>("nil"), reinterpret_cast<xmlChar const *>(NS_SCHEMA_INSTANCE));
            if (sNil == nullptr)
                throw FrameworkException(JFW_E_ERROR, sExcMsg);
            if (xmlStrcmp(sNil, reinterpret_cast<xmlChar const *>("false")) == 0)
            {
                if (! m_JRELocations)
                    m_JRELocations = boost::optional<std::vector<OUString> >(
                        std::vector<OUString>());

                xmlNode * pLoc = cur->children;
                while (pLoc != nullptr)
                {
                    if (xmlStrcmp(pLoc->name, reinterpret_cast<xmlChar const *>("location")) == 0)
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

OString NodeJava::getSettingsPath() const
{
    OString ret;
    switch (m_layer)
    {
    case USER: ret = getUserSettingsPath(); break;
    case SHARED: ret = getSharedSettingsPath(); break;
    default:
        OSL_FAIL("[Java framework] NodeJava::getSettingsPath()");
    }
    return ret;
}

OUString NodeJava::getSettingsURL() const
{
    OUString ret;
    switch (m_layer)
    {
    case USER: ret = BootParams::getUserData(); break;
    case SHARED: ret = BootParams::getSharedData(); break;
    default:
        OSL_FAIL("[Java framework] NodeJava::getSettingsURL()");
    }
    return ret;
}

bool NodeJava::prepareSettingsDocument() const
{
    OString sExcMsg(
        "[Java framework] Error in function prepareSettingsDocument"
        " (elements.cxx).");
    if (!createSettingsDocument())
    {
        return false;
    }
    OString sSettings = getSettingsPath();
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
    return true;
}

void NodeJava::write() const
{
    OString sExcMsg("[Java framework] Error in function NodeJava::writeSettings "
                         "(elements.cxx).");
    CXmlDocPtr docUser;
    CXPathContextPtr contextUser;
    CXPathObjectPtr pathObj;

    if (!prepareSettingsDocument())
    {
        SAL_INFO("jfw.level1", "no path to write settings document to");
        return;
    }

    //Read the user elements
    OString sSettingsPath = getSettingsPath();
    docUser = xmlParseFile(sSettingsPath.getStr());
    if (docUser == nullptr)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);
    contextUser = xmlXPathNewContext(docUser);
    if (xmlXPathRegisterNs(contextUser, reinterpret_cast<xmlChar const *>("jf"),
        reinterpret_cast<xmlChar const *>(NS_JAVA_FRAMEWORK)) == -1)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);

    xmlNode * root = xmlDocGetRootElement(docUser);
    //Get xsi:nil namespace
    xmlNs* nsXsi = xmlSearchNsByHref(docUser,
                             root,
                             reinterpret_cast<xmlChar const *>(NS_SCHEMA_INSTANCE));

    //set the <enabled> element
    //The element must exist
    if (m_enabled)
    {
        OString sExpression= OString(
            "/jf:java/jf:enabled");
        pathObj = xmlXPathEvalExpression(reinterpret_cast<xmlChar const *>(sExpression.getStr()),
                                         contextUser);
        if ( ! pathObj || xmlXPathNodeSetIsEmpty(pathObj->nodesetval))
            throw FrameworkException(JFW_E_ERROR, sExcMsg);

        xmlNode * nodeEnabled = pathObj->nodesetval->nodeTab[0];
        xmlSetNsProp(nodeEnabled,
                     nsXsi,
                     reinterpret_cast<xmlChar const *>("nil"),
                     reinterpret_cast<xmlChar const *>("false"));

        if (m_enabled == boost::optional<sal_Bool>(true))
            xmlNodeSetContent(nodeEnabled,reinterpret_cast<xmlChar const *>("true"));
        else
            xmlNodeSetContent(nodeEnabled,reinterpret_cast<xmlChar const *>("false"));
    }

    //set the <userClassPath> element
    //The element must exist
    if (m_userClassPath)
    {
        OString sExpression= OString(
            "/jf:java/jf:userClassPath");
        pathObj = xmlXPathEvalExpression(reinterpret_cast<xmlChar const *>(sExpression.getStr()),
                                         contextUser);
        if ( ! pathObj || xmlXPathNodeSetIsEmpty(pathObj->nodesetval))
            throw FrameworkException(JFW_E_ERROR, sExcMsg);

        xmlNode * nodeEnabled = pathObj->nodesetval->nodeTab[0];
        xmlSetNsProp(nodeEnabled, nsXsi, reinterpret_cast<xmlChar const *>("nil"),reinterpret_cast<xmlChar const *>("false"));
        xmlNodeSetContent(nodeEnabled,static_cast<xmlChar*>(CXmlCharPtr(*m_userClassPath)));
    }

    //set <javaInfo> element
    if (m_javaInfo)
    {
        OString sExpression= OString(
            "/jf:java/jf:javaInfo");
        pathObj = xmlXPathEvalExpression(reinterpret_cast<xmlChar const *>(sExpression.getStr()),
                                                contextUser);
        if ( ! pathObj || xmlXPathNodeSetIsEmpty(pathObj->nodesetval))
            throw FrameworkException(JFW_E_ERROR, sExcMsg);
        m_javaInfo->writeToNode(
            docUser, pathObj->nodesetval->nodeTab[0]);
    }

    //set <vmParameters> element
    if (m_vmParameters)
    {
        OString sExpression= OString(
            "/jf:java/jf:vmParameters");
        pathObj = xmlXPathEvalExpression(reinterpret_cast<xmlChar const *>(sExpression.getStr()),
                                         contextUser);
        if ( ! pathObj || xmlXPathNodeSetIsEmpty(pathObj->nodesetval))
            throw FrameworkException(JFW_E_ERROR, sExcMsg);
        xmlNode* vmParameters = pathObj->nodesetval->nodeTab[0];
        //set xsi:nil = false;
        xmlSetNsProp(vmParameters, nsXsi,reinterpret_cast<xmlChar const *>("nil"),
                     reinterpret_cast<xmlChar const *>("false"));

        //remove option elements
        xmlNode* cur = vmParameters->children;
        while (cur != nullptr)
        {
            xmlNode* lastNode = cur;
            cur = cur->next;
            xmlUnlinkNode(lastNode);
            xmlFreeNode(lastNode);
        }
        //add a new line after <vmParameters>
        if (!m_vmParameters->empty())
        {
            xmlNode * nodeCrLf = xmlNewText(reinterpret_cast<xmlChar const *>("\n"));
            xmlAddChild(vmParameters, nodeCrLf);
        }

        for (auto const & vmParameter : *m_vmParameters)
        {
            xmlNewTextChild(vmParameters, nullptr, reinterpret_cast<xmlChar const *>("param"),
                            CXmlCharPtr(vmParameter));
            //add a new line
            xmlNode * nodeCrLf = xmlNewText(reinterpret_cast<xmlChar const *>("\n"));
            xmlAddChild(vmParameters, nodeCrLf);
        }
    }

    //set <jreLocations> element
    if (m_JRELocations)
    {
        OString sExpression= OString(
            "/jf:java/jf:jreLocations");
        pathObj = xmlXPathEvalExpression(reinterpret_cast<xmlChar const *>(sExpression.getStr()),
                                         contextUser);
        if ( ! pathObj || xmlXPathNodeSetIsEmpty(pathObj->nodesetval))
            throw FrameworkException(JFW_E_ERROR, sExcMsg);
        xmlNode* jreLocationsNode = pathObj->nodesetval->nodeTab[0];
        //set xsi:nil = false;
        xmlSetNsProp(jreLocationsNode, nsXsi,reinterpret_cast<xmlChar const *>("nil"),
                     reinterpret_cast<xmlChar const *>("false"));

        //remove option elements
        xmlNode* cur = jreLocationsNode->children;
        while (cur != nullptr)
        {
            xmlNode* lastNode = cur;
            cur = cur->next;
            xmlUnlinkNode(lastNode);
            xmlFreeNode(lastNode);
        }
        //add a new line after <vmParameters>
        if (!m_JRELocations->empty())
        {
            xmlNode * nodeCrLf = xmlNewText(reinterpret_cast<xmlChar const *>("\n"));
            xmlAddChild(jreLocationsNode, nodeCrLf);
        }

        for (auto const & JRELocation : *m_JRELocations)
        {
            xmlNewTextChild(jreLocationsNode, nullptr, reinterpret_cast<xmlChar const *>("location"),
                            CXmlCharPtr(JRELocation));
            //add a new line
            xmlNode * nodeCrLf = xmlNewText(reinterpret_cast<xmlChar const *>("\n"));
            xmlAddChild(jreLocationsNode, nodeCrLf);
        }
    }

    if (xmlSaveFormatFile(sSettingsPath.getStr(), docUser, 1) == -1)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);
}

void NodeJava::setEnabled(bool bEnabled)
{
    m_enabled =  boost::optional<sal_Bool>(bEnabled);
}


void NodeJava::setUserClassPath(const OUString & sClassPath)
{
    m_userClassPath = boost::optional<OUString>(sClassPath);
}

void NodeJava::setJavaInfo(const JavaInfo * pInfo, bool bAutoSelect)
{
    if (!m_javaInfo)
        m_javaInfo = boost::optional<CNodeJavaInfo>(CNodeJavaInfo());
    m_javaInfo->bAutoSelect = bAutoSelect;
    m_javaInfo->bNil = false;

    if (pInfo != nullptr)
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
        m_javaInfo->sVendor.clear();
        m_javaInfo->sLocation.clear();
        m_javaInfo->sVersion.clear();
        m_javaInfo->nFeatures = 0;
        m_javaInfo->nRequirements = 0;
        m_javaInfo->arVendorData = rtl::ByteSequence();
    }
}

void NodeJava::setVmParameters(std::vector<OUString> const & arOptions)
{
    m_vmParameters = boost::optional<std::vector<OUString> >(arOptions);
}

void NodeJava::addJRELocation(OUString const & sLocation)
{
    if (!m_JRELocations)
        m_JRELocations = boost::optional<std::vector<OUString> >(
            std::vector<OUString> ());
     //only add the path if not already present
    std::vector<OUString>::const_iterator it =
        std::find(m_JRELocations->begin(), m_JRELocations->end(), sLocation);
    if (it == m_JRELocations->end())
        m_JRELocations->push_back(sLocation);
}

jfw::FileStatus NodeJava::checkSettingsFileStatus(OUString const & sURL)
{
    jfw::FileStatus ret = FILE_DOES_NOT_EXIST;

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

bool NodeJava::createSettingsDocument() const
{
    const OUString sURL = getSettingsURL();
    if (sURL.isEmpty())
    {
        return false;
    }
    //make sure there is a user directory
    OString sExcMsg("[Java framework] Error in function createSettingsDocument "
                         "(elements.cxx).");
    // check if javasettings.xml already exist
    if (FILE_OK == checkSettingsFileStatus(sURL))
        return true;

    //make sure that the directories are created in case they do not exist
    FileBase::RC rcFile = Directory::createPath(getDirFromFile(sURL));
    if (rcFile != FileBase::E_EXIST && rcFile != FileBase::E_None)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);

    //javasettings.xml does not exist yet
    CXmlDocPtr doc(xmlNewDoc(reinterpret_cast<xmlChar const *>("1.0")));
    if (! doc)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);

    //Create the root element and name spaces
    xmlNodePtr root =   xmlNewDocNode(
        doc, nullptr, reinterpret_cast<xmlChar const *>("java"), reinterpret_cast<xmlChar const *>("\n"));

    if (root == nullptr)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);

    if (xmlNewNs(root, reinterpret_cast<xmlChar const *>(NS_JAVA_FRAMEWORK),nullptr) == nullptr)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);
    if (xmlNewNs(root,reinterpret_cast<xmlChar const *>(NS_SCHEMA_INSTANCE),reinterpret_cast<xmlChar const *>("xsi")) == nullptr)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);
    xmlDocSetRootElement(doc,  root);

    //Create a comment
    xmlNodePtr com = xmlNewComment(
        reinterpret_cast<xmlChar const *>("This is a generated file. Do not alter this file!"));
    if (com == nullptr)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);

    if (xmlAddPrevSibling(root, com) == nullptr)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);

    const OString path = getSettingsPath();
    if (xmlSaveFormatFileEnc(path.getStr(), doc,"UTF-8", 1) == -1)
         throw FrameworkException(JFW_E_ERROR, sExcMsg);
    return true;
}


CNodeJavaInfo::CNodeJavaInfo() :
    m_bEmptyNode(false), bNil(true), bAutoSelect(true),
    nFeatures(0), nRequirements(0)
{
}

void CNodeJavaInfo::loadFromNode(xmlDoc * pDoc, xmlNode * pJavaInfo)
{
    OString sExcMsg("[Java framework] Error in function NodeJavaInfo::loadFromNode "
                         "(elements.cxx).");

    OSL_ASSERT(pJavaInfo && pDoc);
    if (pJavaInfo->children == nullptr)
        return;
    //Get the xsi:nil attribute;
    CXmlCharPtr sNil;
    sNil = xmlGetNsProp(
        pJavaInfo, reinterpret_cast<xmlChar const *>("nil"), reinterpret_cast<xmlChar const *>(NS_SCHEMA_INSTANCE));
    if ( ! sNil)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);

    if (xmlStrcmp(sNil, reinterpret_cast<xmlChar const *>("true")) == 0)
        bNil = true;
    else if (xmlStrcmp(sNil, reinterpret_cast<xmlChar const *>("false")) == 0)
        bNil = false;
    else
        throw FrameworkException(JFW_E_ERROR, sExcMsg);
    if (bNil)
        return;

    //Get javaInfo@manuallySelected attribute
    CXmlCharPtr sAutoSelect;
    sAutoSelect = xmlGetProp(
        pJavaInfo, reinterpret_cast<xmlChar const *>("autoSelect"));
    if ( ! sAutoSelect)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);

    if (xmlStrcmp(sAutoSelect, reinterpret_cast<xmlChar const *>("true")) == 0)
        bAutoSelect = true;
    else if (xmlStrcmp(sAutoSelect, reinterpret_cast<xmlChar const *>("false")) == 0)
        bAutoSelect = false;
    else
        throw FrameworkException(JFW_E_ERROR, sExcMsg);

    xmlNode * cur = pJavaInfo->children;

    while (cur != nullptr)
    {
        if (xmlStrcmp(cur->name, reinterpret_cast<xmlChar const *>("vendor")) == 0)
        {
            CXmlCharPtr xmlVendor;
            xmlVendor = xmlNodeListGetString(
                pDoc, cur->children, 1);
            if (! xmlVendor)
                return;
            sVendor = xmlVendor;
        }
        else if (xmlStrcmp(cur->name, reinterpret_cast<xmlChar const *>("location")) == 0)
        {
            CXmlCharPtr xmlLocation;
            xmlLocation = xmlNodeListGetString(
                pDoc, cur->children, 1);
            sLocation = xmlLocation;
        }
        else if (xmlStrcmp(cur->name, reinterpret_cast<xmlChar const *>("version")) == 0)
        {
            CXmlCharPtr xmlVersion;
            xmlVersion = xmlNodeListGetString(
                pDoc, cur->children, 1);
            sVersion = xmlVersion;
        }
        else if (xmlStrcmp(cur->name, reinterpret_cast<xmlChar const *>("features"))== 0)
        {
            CXmlCharPtr xmlFeatures;
            xmlFeatures = xmlNodeListGetString(
                    pDoc, cur->children, 1);
            OUString sFeatures = xmlFeatures;
            nFeatures = sFeatures.toInt64(16);
        }
        else if (xmlStrcmp(cur->name, reinterpret_cast<xmlChar const *>("requirements")) == 0)
        {
            CXmlCharPtr xmlRequire;
            xmlRequire = xmlNodeListGetString(
                pDoc, cur->children, 1);
            OUString sRequire = xmlRequire;
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
        else if (xmlStrcmp(cur->name, reinterpret_cast<xmlChar const *>("vendorData")) == 0)
        {
            CXmlCharPtr xmlData;
            xmlData = xmlNodeListGetString(
                pDoc, cur->children, 1);
            xmlChar* _data = static_cast<xmlChar*>(xmlData);
            if (_data)
            {
                rtl::ByteSequence seq(reinterpret_cast<sal_Int8*>(_data), strlen(reinterpret_cast<char*>(_data)));
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
                               reinterpret_cast<xmlChar const *>("vendorUpdate"));
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
    OString sUpdated = getElementUpdated();

    xmlSetProp(pJavaInfoNode, reinterpret_cast<xmlChar const *>("vendorUpdate"),
               reinterpret_cast<xmlChar const *>(sUpdated.getStr()));

    //javaInfo@autoSelect
    xmlSetProp(pJavaInfoNode, reinterpret_cast<xmlChar const *>("autoSelect"),
               reinterpret_cast<xmlChar const *>(bAutoSelect ? "true" : "false"));

    //Set xsi:nil in javaInfo element to false
    //the xmlNs pointer must not be destroyed
    xmlNs* nsXsi = xmlSearchNsByHref(pDoc,
                             pJavaInfoNode,
                             reinterpret_cast<xmlChar const *>(NS_SCHEMA_INSTANCE));

    xmlSetNsProp(pJavaInfoNode,
                 nsXsi,
                 reinterpret_cast<xmlChar const *>("nil"),
                 reinterpret_cast<xmlChar const *>("false"));

    //Delete the children of JavaInfo
    xmlNode* cur = pJavaInfoNode->children;
    while (cur != nullptr)
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
    xmlNode * nodeCrLf = xmlNewText(reinterpret_cast<xmlChar const *>("\n"));
    xmlAddChild(pJavaInfoNode, nodeCrLf);

    //Create the vendor element
    xmlNewTextChild(pJavaInfoNode, nullptr, reinterpret_cast<xmlChar const *>("vendor"),
                    CXmlCharPtr(sVendor));
    //add a new line for better readability
    nodeCrLf = xmlNewText(reinterpret_cast<xmlChar const *>("\n"));
    xmlAddChild(pJavaInfoNode, nodeCrLf);

    //Create the location element
    xmlNewTextChild(pJavaInfoNode, nullptr, reinterpret_cast<xmlChar const *>("location"),
                    CXmlCharPtr(sLocation));
    //add a new line for better readability
    nodeCrLf = xmlNewText(reinterpret_cast<xmlChar const *>("\n"));
    xmlAddChild(pJavaInfoNode, nodeCrLf);

    //Create the version element
    xmlNewTextChild(pJavaInfoNode, nullptr, reinterpret_cast<xmlChar const *>("version"),
                    CXmlCharPtr(sVersion));
    //add a new line for better readability
    nodeCrLf = xmlNewText(reinterpret_cast<xmlChar const *>("\n"));
    xmlAddChild(pJavaInfoNode, nodeCrLf);

    //Create the features element
    OUString sFeatures = OUString::number(
        nFeatures, 16);
    xmlNewTextChild(pJavaInfoNode, nullptr, reinterpret_cast<xmlChar const *>("features"),
                    CXmlCharPtr(sFeatures));
    //add a new line for better readability
    nodeCrLf = xmlNewText(reinterpret_cast<xmlChar const *>("\n"));
    xmlAddChild(pJavaInfoNode, nodeCrLf);


    //Create the requirements element
    OUString sRequirements = OUString::number(
         nRequirements, 16);
    xmlNewTextChild(pJavaInfoNode, nullptr, reinterpret_cast<xmlChar const *>("requirements"),
                    CXmlCharPtr(sRequirements));
    //add a new line for better readability
    nodeCrLf = xmlNewText(reinterpret_cast<xmlChar const *>("\n"));
    xmlAddChild(pJavaInfoNode, nodeCrLf);


    //Create the features element
    rtl::ByteSequence data = encodeBase16(arVendorData);
    xmlNode* dataNode = xmlNewChild(pJavaInfoNode, nullptr,
                                    reinterpret_cast<xmlChar const *>("vendorData"),
                                    reinterpret_cast<xmlChar const *>(""));
    xmlNodeSetContentLen(dataNode,
                         reinterpret_cast<xmlChar*>(data.getArray()), data.getLength());
    //add a new line for better readability
    nodeCrLf = xmlNewText(reinterpret_cast<xmlChar const *>("\n"));
    xmlAddChild(pJavaInfoNode, nodeCrLf);
}

std::unique_ptr<JavaInfo> CNodeJavaInfo::makeJavaInfo() const
{
    if (bNil || m_bEmptyNode)
        return std::unique_ptr<JavaInfo>();
    return std::unique_ptr<JavaInfo>(
        new JavaInfo{
            sVendor, sLocation, sVersion, nFeatures, nRequirements,
            arVendorData});
}


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
        m_bEnabled = true;

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


::std::vector< OString> MergedSettings::getVmParametersUtf8() const
{
    ::std::vector< OString> ret;
    for (auto const & vmParam : m_vmParams)
    {
        ret.push_back( OUStringToOString(vmParam, RTL_TEXTENCODING_UTF8));
    }
    return ret;
}


std::unique_ptr<JavaInfo> MergedSettings::createJavaInfo() const
{
    return m_javaInfo.makeJavaInfo();
}
#ifdef _WIN32
bool MergedSettings::getJavaInfoAttrAutoSelect() const
{
    return m_javaInfo.bAutoSelect;
}
#endif
void MergedSettings::getVmParametersArray(std::vector<OUString> * parParams)
    const
{
    assert(parParams != nullptr);
    osl::MutexGuard guard(FwkMutex::get());

    *parParams = m_vmParams;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
