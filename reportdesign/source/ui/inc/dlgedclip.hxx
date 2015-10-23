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

#ifndef INCLUDED_REPORTDESIGN_SOURCE_UI_INC_DLGEDCLIP_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_UI_INC_DLGEDCLIP_HXX

#include <svtools/transfer.hxx>
#include <com/sun/star/beans/NamedValue.hpp>

namespace rptui
{

// OReportExchange

/** \class OReportExchange
 * \brief defines a clipboard format for copying selection elements.
 * \ingroup reportdesign_source_ui_report
 *
 *
 */
class OReportExchange : public TransferableHelper
{
public:
    typedef css::uno::Sequence< css::beans::NamedValue > TSectionElements;

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
    static bool         canExtract(const DataFlavorExVector& _rFlavors);

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
    static SotClipboardFormatId getDescriptorFormatId();

protected:
    // TransferableHelper overridables
    virtual void        AddSupportedFormats() override;
    virtual bool GetData( const css::datatransfer::DataFlavor& rFlavor, const OUString& rDestDoc ) override;
private:
    TSectionElements    m_aCopyElements;
};
}
#endif // INCLUDED_REPORTDESIGN_SOURCE_UI_INC_DLGEDCLIP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
