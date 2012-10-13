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
#ifndef INCLUDED_JVMFWK_ELEMENTS_HXX
#define INCLUDED_JVMFWK_ELEMENTS_HXX

#include <vector>
#include "jvmfwk/framework.h"
#include "fwkutil.hxx"
#include "rtl/ustring.hxx"
#include "rtl/byteseq.hxx"
#include "libxml/parser.h"
#include "boost/optional.hpp"

#define NS_JAVA_FRAMEWORK "http://openoffice.org/2004/java/framework/1.0"
#define NS_SCHEMA_INSTANCE "http://www.w3.org/2001/XMLSchema-instance"

namespace jfw
{

/** gets the value of the updated element from the javavendors.xml.
 */
rtl::OString getElementUpdated();

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
    ~CNodeJavaInfo();

    /** if true, then javaInfo is empty. When writeToNode is called
        then all child elements are deleted.
     */
    bool m_bEmptyNode;
    /** Contains the value of the <updated> element of
        the javavendors.xml after loadFromNode was called.
        It is not used, when the javaInfo node is written.
        see writeToNode
     */
    ::rtl::OString sAttrVendorUpdate;
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
    ::rtl::OUString sVendor;
    ::rtl::OUString sLocation;
    ::rtl::OUString sVersion;
    sal_uInt64 nFeatures;
    sal_uInt64 nRequirements;
    ::rtl::ByteSequence arVendorData;

    /** reads the node /java/javaInfo.
        If javaInfo@xsi:nil = true then member bNil is set to true
        an no further elements are read.
     */
    void loadFromNode(xmlDoc * pDoc,xmlNode * pJavaInfo);
    /** The attribut nil will be set to false. The function gets the value
        javaSettings/updated from the javavendors.xml and writes it to
        javaInfo@vendorUpdate in javasettings.xml
     */
    void writeToNode(xmlDoc * pDoc, xmlNode * pJavaInfo) const;

    /** returns NULL if javaInfo is nil.
     */
    JavaInfo * makeJavaInfo() const;
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
        settings file at the possition determined by the bootstrap parameters
        (UNO_JAVA_JFW_USER_DATA, UNO_JAVA_JFW_SHARED_DATA) and m_layer, unless
        the file already exists (see createSettingsDocument).

        @return
        JFW_E_CONFIG_READWRITE
    */
    void prepareSettingsDocument() const;

    /** helper function for prepareSettingsDocument.
    */
    void createSettingsDocument() const;

    /** returns the system path to the data file which is to be used. The value
        depends on the the member m_layer and the bootstrap parameters
        UNO_JAVA_JFW_USER_DATA and UNO_JAVA_JFW_SHARED_DATA.
    */
    ::rtl::OString getSettingsPath() const;

    /** returns the file URL to the data file which is to be used. See getSettingsPath.
    */
    ::rtl::OUString getSettingsURL() const;

    /** Verifies if the respective settings file exist.
     */
    jfw::FileStatus checkSettingsFileStatus() const;

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
    boost::optional< ::rtl::OUString> m_userClassPath;
    /** User configurable option.  /java/javaInfo
        If /java/javaInfo@xsi:nil == true then the value is uninitialized
        after a call to load.
     */
    boost::optional<CNodeJavaInfo> m_javaInfo;
    /** User configurable option. /java/vmParameters
        If /java/vmParameters@xsi:nil == true then the value is uninitialized
        after a call to load.
    */
    boost::optional< ::std::vector< ::rtl::OUString> > m_vmParameters;
    /** User configurable option. /java/jreLocations
        If /java/jreLocaltions@xsi:nil == true then the value is uninitialized
        after a call to load.
    */
    boost::optional< ::std::vector< ::rtl::OUString> > m_JRELocations;

public:

    explicit NodeJava(Layer theLayer);

    /** sets m_enabled.
        /java/enabled@xsi:nil will be set to false when write is called.
     */
    void setEnabled(sal_Bool bEnabled);

    /** sets m_sUserClassPath. See setEnabled.
     */
    void setUserClassPath(const ::rtl::OUString & sClassPath);

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
    void setVmParameters(rtl_uString  * * arParameters, sal_Int32 size);

    /** sets the /java/jreLocations/location elements.
        When this method is called then all previous values are removed
        and replaced by those in arParamters.
        /java/jreLocations@xsi:nil will be set to true write() is called.
     */
    void setJRELocations(rtl_uString  * * arParameters, sal_Int32 size);

    /** adds a location to the already existing locations.
        Note: call load() before, then add the location and then call write().
    */
    void addJRELocation(rtl_uString * sLocation);

    /** writes the data to user settings.
     */
    void write() const;

    /** load the values of the settings file.
     */
    void load();

    /** returns the value of the element /java/enabled
     */
    const boost::optional<sal_Bool> & getEnabled() const;
    /** returns the value of the element /java/userClassPath.
     */
    const boost::optional< ::rtl::OUString> & getUserClassPath() const;

    /** returns the value of the element /java/javaInfo.
     */
    const boost::optional<CNodeJavaInfo> & getJavaInfo() const;

    /** returns the parameters from the element /java/vmParameters/param.
     */
    const boost::optional< ::std::vector< ::rtl::OUString> > & getVmParameters() const;

    /** returns the parameters from the element /java/jreLocations/location.
     */
    const boost::optional< ::std::vector< ::rtl::OUString> > & getJRELocations() const;
};

/** merges the settings for shared, user and installation during construction.
    The class uses a simple merge mechanism for the javasettings.xml files in share and
    user. The following elements completly overwrite the corresponding elements
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
class MergedSettings
{
private:
    const MergedSettings& operator = (MergedSettings&);
    MergedSettings(MergedSettings&);

    void merge(const NodeJava & share, const NodeJava & user);

    bool m_bEnabled;

    ::rtl::OUString m_sClassPath;

    ::std::vector< ::rtl::OUString> m_vmParams;

    ::std::vector< ::rtl::OUString> m_JRELocations;

    CNodeJavaInfo m_javaInfo;

public:
    MergedSettings();
    virtual ~MergedSettings();

    /** the default is true.
     */
    bool getEnabled() const;

    const ::rtl::OUString & getUserClassPath() const;

    ::std::vector< ::rtl::OString> getVmParametersUtf8() const;
    /** returns a JavaInfo structure representing the node
        /java/javaInfo. Every time a new JavaInfo structure is created
        which needs to be freed by the caller.
        If both, user and share settings are nil, then NULL is returned.
    */
    JavaInfo * createJavaInfo() const;

    /** returns the value of the attribute /java/javaInfo[@vendorUpdate].
     */
    ::rtl::OString const & getJavaInfoAttrVendorUpdate() const;

#ifdef WNT
    /** returns the javaInfo@autoSelect attribute.
        Before calling this function loadFromSettings must be called.
        It uses the javaInfo@autoSelect attribute  to determine
        the return value;
     */
    bool getJavaInfoAttrAutoSelect() const;
#endif

    /** returns an array.
        Caller must free the strings and the array.
     */
    void getVmParametersArray(rtl_uString *** parParameters, sal_Int32 * size) const;

    /** returns an array.
        Caller must free the strings and the array.
     */
    void getJRELocations(rtl_uString *** parLocations, sal_Int32 * size) const;

    const ::std::vector< ::rtl::OUString> & getJRELocations() const;
};


class VersionInfo
{
    ::std::vector< ::rtl::OUString> vecExcludeVersions;
    rtl_uString ** arVersions;

public:
    VersionInfo();
    ~VersionInfo();

    void addExcludeVersion(const ::rtl::OUString& sVersion);

    ::rtl::OUString sMinVersion;
    ::rtl::OUString sMaxVersion;

    /** The caller DOES NOT get ownership of the strings. That is he
        does not need to release the strings.
        The array exists as long as this object exists.
    */

    rtl_uString** getExcludeVersions();
    sal_Int32 getExcludeVersionSize();
};

struct PluginLibrary
{
    PluginLibrary()
    {
    }
    PluginLibrary(rtl::OUString vendor,::rtl::OUString path) :
        sVendor(vendor), sPath(path)
    {
    }
    /** contains the vendor string which is later userd in the xml API
     */
    ::rtl::OUString sVendor;
    /** File URL the plug-in library
     */
    ::rtl::OUString sPath;
};

} //end namespace
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
