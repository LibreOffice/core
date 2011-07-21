/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include <resourcemodel/LoggedResources.hxx>
#include <rtl/ustring.hxx>
namespace writerfilter {
namespace dmapper {
class FFDataHandler : public LoggedProperties
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
    const rtl::OUString & getName() const;

    // member: helpText
    const rtl::OUString & getHelpText() const;

    // member: statusText
    const rtl::OUString & getStatusText() const;

    // member: checkboxHeight
    sal_uInt32 getCheckboxHeight() const;

    // member: checkboxAutoHeight
    bool getCheckboxAutoHeight() const;

    // member: checkboxChecked
    bool getCheckboxChecked() const;

    // member: dropDownResult
    const rtl::OUString & getDropDownResult() const;

    // member: dropDownEntries
    const DropDownEntries_t & getDropDownEntries() const;

    // member: textDefault
    const rtl::OUString & getTextDefault() const;

    // sprm
    void resolveSprm(Sprm & r_sprm);

private:
    rtl::OUString m_sName;
    rtl::OUString m_sHelpText;
    rtl::OUString m_sStatusText;
    sal_uInt32 m_nCheckboxHeight;
    bool m_bCheckboxAutoHeight;
    bool m_bCheckboxChecked;
    rtl::OUString m_sDropDownResult;
    DropDownEntries_t m_DropDownEntries;
    rtl::OUString m_sTextDefault;

    // sprm
    void lcl_sprm(Sprm & r_sprm);

    // attribute
    void lcl_attribute(Id name, Value & val);
};


}}
#endif //INCLUDED_FFDataHandler_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
