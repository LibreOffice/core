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
#ifndef INCLUDED_JVMFWK_SOURCE_ELEMENTS_HXX
#define INCLUDED_JVMFWK_SOURCE_ELEMENTS_HXX

#include <sal/config.h>

#include <memory>
#include <vector>
#include <jvmfwk/framework.hxx>
#include "fwkutil.hxx"
#include <rtl/ustring.hxx>
#include <rtl/byteseq.hxx>
#include <libxml/parser.h>
#include <boost/optional.hpp>

#define NS_JAVA_FRAMEWORK "http://openoffice.org/2004/java/framework/1.0"
#define NS_SCHEMA_INSTANCE "http://www.w3.org/2001/XMLSchema-instance"

namespace jfw
{

/** gets the value of the updated element from the javavendors.xml.
 */
OString getElementUpdated();

/** create the child elements within the root structure for each platform.

    @param bNeedsSave
    [out]If true then the respective structure of elements was added and the
    document needs to be saved.
 */
void createSettingsStructure(
    xmlDoc * document, bool * bNeedsSave);


/** represents the settings saved in the /java/javaInfo element.
    It is used within class NodeJava which determines the settings
    file.
*/
class CNodeJavaInfo
{
public:
    CNodeJavaInfo();

    /** if true, then javaInfo is empty. When writeToNode is called
        then all child elements are deleted.
     */
    bool m_bEmptyNode;
    /** Contains the value of the <updated> element of
        the javavendors.xml after loadFromNode was called.
        It is not used, when the javaInfo node is written.
        see writeToNode
     */
    OString sAttrVendorUpdate;
    /** contains the nil value of the /java/javaInfo@xsi:nil attribute.
        Default is true;
     */
    bool bNil;
    /** contains the value of the /java/javaInfo@autoSelect attribute.
        Default is true. If it is false then the user has modified the JRE
        selection by actively choosing a JRE from the options dialog. That is,
        the function jfw_setSelectedJRE was called. Contrary, the function
        jfw_findAndSelectJRE sets the attribute to true.
     */
    bool bAutoSelect;
    OUString sVendor;
    OUString sLocation;
    OUString sVersion;
    sal_uInt64 nFeatures;
    sal_uInt64 nRequirements;
    ::rtl::ByteSequence arVendorData;

    /** reads the node /java/javaInfo.
        If javaInfo@xsi:nil = true then member bNil is set to true
        an no further elements are read.
     */
    void loadFromNode(xmlDoc * pDoc,xmlNode * pJavaInfo);
    /** The attribute nil will be set to false. The function gets the value
        javaSettings/updated from the javavendors.xml and writes it to
        javaInfo@vendorUpdate in javasettings.xml
     */
    void writeToNode(xmlDoc * pDoc, xmlNode * pJavaInfo) const;

    /** returns NULL if javaInfo is nil.
     */
    std::unique_ptr<JavaInfo> makeJavaInfo() const;
};

/** this class represents the java settings  based on a particular
    settings file.

    Which settings file is used is determined by the value passed into the
    constructor and the values of the bootstrap parameters
    UNO_JAVA_JFW_USER_DATA and UNO_JAVA_JFW_SHARED_DATA.

    The method load reads the data from the settings file.
    The method write stores the data into the settings file.
 */
class NodeJava
{
public:
    enum Layer { USER, SHARED };
private:

    /** creates settings file and fills it with default values.

        When this function is called then it creates the
        settings file at the position determined by the bootstrap parameters
        (UNO_JAVA_JFW_USER_DATA, UNO_JAVA_JFW_SHARED_DATA) and m_layer, unless
        the file already exists (see createSettingsDocument).

        @return
        JFW_E_CONFIG_READWRITE
    */
    bool prepareSettingsDocument() const;

    /** helper function for prepareSettingsDocument.
    */
    bool createSettingsDocument() const;

    /** returns the system path to the data file which is to be used. The value
        depends on the member m_layer and the bootstrap parameters
        UNO_JAVA_JFW_USER_DATA and UNO_JAVA_JFW_SHARED_DATA.
    */
    OString getSettingsPath() const;

    /** returns the file URL to the data file which is to be used. See getSettingsPath.
    */
    OUString getSettingsURL() const;

    /** Verifies if the respective settings file exist.
     */
    static jfw::FileStatus checkSettingsFileStatus(OUString const & sURL);

    /** Determines the layer for which the instance the loads and writes the
        data.
    */
    Layer m_layer;

    /** User configurable option.  /java/enabled
        If /java/enabled@xsi:nil == true then the value will be uninitialized
        after a call to load().
    */
    boost::optional<sal_Bool> m_enabled;

    /** User configurable option. /java/userClassPath
        If /java/userClassPath@xsi:nil == true then the value is uninitialized
        after a call to load().
    */
    boost::optional< OUString> m_userClassPath;
    /** User configurable option.  /java/javaInfo
        If /java/javaInfo@xsi:nil == true then the value is uninitialized
        after a call to load.
     */
    boost::optional<CNodeJavaInfo> m_javaInfo;
    /** User configurable option. /java/vmParameters
        If /java/vmParameters@xsi:nil == true then the value is uninitialized
        after a call to load.
    */
    boost::optional< ::std::vector< OUString> > m_vmParameters;
    /** User configurable option. /java/jreLocations
        If /java/jreLocaltions@xsi:nil == true then the value is uninitialized
        after a call to load.
    */
    boost::optional< ::std::vector< OUString> > m_JRELocations;

public:

    explicit NodeJava(Layer theLayer);

    /** sets m_enabled.
        /java/enabled@xsi:nil will be set to false when write is called.
     */
    void setEnabled(bool bEnabled);

    /** sets m_sUserClassPath. See setEnabled.
     */
    void setUserClassPath(const OUString & sClassPath);

    /** sets m_aInfo. See setEnabled.
        @param bAutoSelect
        true- called by jfw_setSelectedJRE
        false called by jfw_findAndSelectJRE
     */
    void setJavaInfo(const JavaInfo * pInfo, bool bAutoSelect);

    /** sets the /java/vmParameters/param elements.
        When this method all previous values are removed and replaced
        by those in arParameters.
        /java/vmParameters@xsi:nil will be set to true when write() is
        called.
     */
    void setVmParameters(std::vector<OUString> const & arParameters);

    /** adds a location to the already existing locations.
        Note: call load() before, then add the location and then call write().
    */
    void addJRELocation(OUString const & sLocation);

    /** writes the data to user settings.
     */
    void write() const;

    /** load the values of the settings file.
     */
    void load();

    /** returns the value of the element /java/enabled
     */
    const boost::optional<sal_Bool> & getEnabled() const { return m_enabled;}
    /** returns the value of the element /java/userClassPath.
     */
    const boost::optional< OUString> & getUserClassPath() const { return m_userClassPath;}

    /** returns the value of the element /java/javaInfo.
     */
    const boost::optional<CNodeJavaInfo> & getJavaInfo() const { return m_javaInfo;}

    /** returns the parameters from the element /java/vmParameters/param.
     */
    const boost::optional< ::std::vector< OUString> > & getVmParameters() const { return m_vmParameters;}

    /** returns the parameters from the element /java/jreLocations/location.
     */
    const boost::optional< ::std::vector< OUString> > & getJRELocations() const { return m_JRELocations;}
};

/** merges the settings for shared, user and installation during construction.
    The class uses a simple merge mechanism for the javasettings.xml files in share and
    user. The following elements completely overwrite the corresponding elements
    from share:
    /java/enabled
    /java/userClassPath
    /java/vmParameters
    /java/jreLocations
    /java/javaInfo

    In case of an installation, the shared and user settings are completely
    disregarded.

    The locations of the different settings files is obtained through the
    bootstrap variables:
    UNO_JAVA_JFW_USER_DATA
    UNO_JAVA_JFW_SHARED_DATA

    The class also determines useful default values for settings which have not been made.
*/
class MergedSettings final
{
private:
    const MergedSettings& operator = (MergedSettings&) = delete;
    MergedSettings(MergedSettings&) = delete;

    void merge(const NodeJava & share, const NodeJava & user);

    bool m_bEnabled;

    OUString m_sClassPath;

    ::std::vector< OUString> m_vmParams;

    ::std::vector< OUString> m_JRELocations;

    CNodeJavaInfo m_javaInfo;

public:
    MergedSettings();
    ~MergedSettings();

    /** the default is true.
     */
    bool getEnabled() const { return m_bEnabled;}

    const OUString & getUserClassPath() const { return m_sClassPath;}

    ::std::vector< OString> getVmParametersUtf8() const;
    /** returns a JavaInfo structure representing the node
        /java/javaInfo. Every time a new JavaInfo structure is created
        which needs to be freed by the caller.
        If both, user and share settings are nil, then NULL is returned.
    */
    std::unique_ptr<JavaInfo> createJavaInfo() const;

    /** returns the value of the attribute /java/javaInfo[@vendorUpdate].
     */
    OString const & getJavaInfoAttrVendorUpdate() const { return m_javaInfo.sAttrVendorUpdate;}

#ifdef _WIN32
    /** returns the javaInfo@autoSelect attribute.
        Before calling this function loadFromSettings must be called.
        It uses the javaInfo@autoSelect attribute  to determine
        the return value;
     */
    bool getJavaInfoAttrAutoSelect() const;
#endif

    void getVmParametersArray(std::vector<OUString> * parParameters) const;

    const ::std::vector< OUString> & getJRELocations() const { return m_JRELocations;}
};


struct VersionInfo
{
    ::std::vector< OUString> vecExcludeVersions;
    OUString sMinVersion;
    OUString sMaxVersion;
};

} //end namespace
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
