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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_NEWDATATYPE_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_NEWDATATYPE_HXX

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>

#include <set>
#include <vector>

//........................................................................
namespace pcr
{
//........................................................................

    //====================================================================
    //= NewDataTypeDialog
    //====================================================================
    class NewDataTypeDialog : public ModalDialog
    {
    private:
        FixedText       m_aLabel;
        Edit            m_aName;
        OKButton        m_aOK;
        CancelButton    m_aCancel;

        ::std::set< OUString >
                        m_aProhibitedNames;

    public:
        NewDataTypeDialog( Window* _pParent, const OUString& _rNameBase, const ::std::vector< OUString >& _rProhibitedNames );

        inline String   GetName() const { return m_aName.GetText(); }

    private:
        DECL_LINK( OnNameModified, void* );
    };

//........................................................................
} // namespace pcr
//........................................................................

#endif // EXTENSIONS_SOURCE_PROPCTRLR_NEWDATATYPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
