/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PropertyMap.hxx,v $
 * $Revision: 1.18 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef INCLUDED_FFDataHandler_HXX
#define INCLUDED_FFDataHandler_HXX
#include <resourcemodel/WW8ResourceModel.hxx>
#include <rtl/ustring.hxx>
namespace writerfilter {
namespace dmapper {
class FFDataHandler : public Properties
{
public:
    // typedefs
    typedef ::boost::shared_ptr<FFDataHandler> Pointer_t;
    typedef ::std::vector<rtl::OUString> DropDownEntries_t;

    // constructor
    FFDataHandler();
    // destructor
    virtual ~FFDataHandler();

    // member: name
    void setName(const rtl::OUString & r_sName);
    const rtl::OUString & getName() const;
    
    // member: enabled
    void setEnabled(bool r_enabled);
    bool getEnabled() const;
    
    // member: calcOnExit
    void setCalcOnExit(bool r_calcOnExit);
    bool getCalcOnExit() const;
    
    // member: entryMacro
    void setEntryMacro(const rtl::OUString & r_sEntryMacro);
    const rtl::OUString & getEntryMacro() const;
    
    // member: exitMacro
    void setExitMacro(const rtl::OUString & r_sExitMacro);
    const rtl::OUString & getExitMacro() const;
    
    // member: helpTextType
    void setHelpTextType(sal_uInt32 r_helpTextType);
    sal_uInt32 getHelpTextType() const;
    
    // member: helpText
    void setHelpText(const rtl::OUString & r_sHelpText);
    const rtl::OUString & getHelpText() const;
    
    // member: statusTextType
    void setStatusTextType(sal_uInt32 r_statusTextType);
    sal_uInt32 getStatusTextType() const;
    
    // member: statusText
    void setStatusText(const rtl::OUString & r_sStatusText);
    const rtl::OUString & getStatusText() const;
    
    // member: checkboxHeight
    void setCheckboxHeight(sal_uInt32 r_checkboxHeight);
    sal_uInt32 getCheckboxHeight() const;
    
    // member: checkboxAutoHeight
    void setCheckboxAutoHeight(bool r_checkboxAutoHeight);
    bool getCheckboxAutoHeight() const;
    
    // member: checkboxDefault
    void setCheckboxDefault(bool r_checkboxDefault);
    bool getCheckboxDefault() const;
    
    // member: checkboxChecked
    void setCheckboxChecked(bool r_checkboxChecked);
    bool getCheckboxChecked() const;
    
    // member: dropDownResult
    void setDropDownResult(const rtl::OUString & r_sDropDownResult);
    const rtl::OUString & getDropDownResult() const;
    
    // member: dropDownDefault
    void setDropDownDefault(const rtl::OUString & r_sDropDownDefault);
    const rtl::OUString & getDropDownDefault() const;
    
    // member: dropDownEntries
    void setDropDownEntries(const DropDownEntries_t & r_dropDownEntries);
    const DropDownEntries_t & getDropDownEntries() const;
    void dropDownEntriesPushBack(const rtl::OUString & r_Element);

    // member: textType
    void setTextType(sal_uInt32 r_textType);
    sal_uInt32 getTextType() const;
    
    // member: textMaxLength
    void setTextMaxLength(sal_uInt32 r_textMaxLength);
    sal_uInt32 getTextMaxLength() const;
    
    // member: textDefault
    void setTextDefault(const rtl::OUString & r_sTextDefault);
    const rtl::OUString & getTextDefault() const;
    
    // member: textFormat
    void setTextFormat(const rtl::OUString & r_sTextFormat);
    const rtl::OUString & getTextFormat() const;
    
    // sprm
    void sprm(Sprm & r_sprm);
    void resolveSprm(Sprm & r_sprm);

    // attribute
    void attribute(Id name, Value & val);

private:
    rtl::OUString m_sName;
    bool m_bEnabled;
    bool m_bCalcOnExit;
    rtl::OUString m_sEntryMacro;
    rtl::OUString m_sExitMacro;
    sal_uInt32 m_nHelpTextType;
    rtl::OUString m_sHelpText;
    sal_uInt32 m_nStatusTextType;
    rtl::OUString m_sStatusText;
    sal_uInt32 m_nCheckboxHeight;
    bool m_bCheckboxAutoHeight;
    bool m_bCheckboxDefault;
    bool m_bCheckboxChecked;
    rtl::OUString m_sDropDownResult;
    rtl::OUString m_sDropDownDefault;
    DropDownEntries_t m_DropDownEntries;
    sal_uInt32 m_nTextType;
    sal_uInt32 m_nTextMaxLength;
    rtl::OUString m_sTextDefault;
    rtl::OUString m_sTextFormat;
};


}}
#endif //INCLUDED_FFDataHandler_HXX
