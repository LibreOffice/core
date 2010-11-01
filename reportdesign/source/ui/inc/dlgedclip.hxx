/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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

#ifndef _REPORT_RPTUICLIP_HXX
#define _REPORT_RPTUICLIP_HXX

#include <svtools/transfer.hxx>
#include <com/sun/star/beans/NamedValue.hpp>

namespace rptui
{
//============================================================================
// OReportExchange
//============================================================================
/** \class OReportExchange
 * \brief defines a clipboard fromat for copying selection elements.
 * \ingroup reportdesign_source_ui_report
 *
 *
 */
class OReportExchange : public TransferableHelper
{
public:
    typedef ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue > TSectionElements;

    /** Constructs a new exchange object with section elements
    *
    * \param _rCopyElements the elements to copy. Each section is one entry. The value must be a sequence of elements.
    * \return
    */
    OReportExchange( const TSectionElements& _rCopyElements);

    /** checks whether or not a descriptor can be extracted from the data flavor vector given
    *
    * \param _rFlavors
            available flavors
    * \return
    */
    static sal_Bool         canExtract(const DataFlavorExVector& _rFlavors);

    /** extract the section elements
    *
    * \param _rData the clipboard data
    * \return the copied elements
    */
    static TSectionElements extractCopies(const TransferableDataHelper& _rData);

    /** returns the format id.
    *
    * \return the registered format id
    */
    static sal_uInt32       getDescriptorFormatId();

protected:
    // TransferableHelper overridables
    virtual void        AddSupportedFormats();
    virtual sal_Bool    GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
private:
    TSectionElements    m_aCopyElements;
};
}
#endif // _REPORT_RPTUICLIP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
