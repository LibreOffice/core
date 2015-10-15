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

#ifndef INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_NEWDATATYPE_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_NEWDATATYPE_HXX

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>

#include <set>
#include <vector>


namespace pcr
{
    //= NewDataTypeDialog
    class NewDataTypeDialog : public ModalDialog
    {
    private:
        VclPtr<Edit>           m_pName;
        VclPtr<OKButton>       m_pOK;

        ::std::set< OUString > m_aProhibitedNames;

    public:
        NewDataTypeDialog(vcl::Window* _pParent, const OUString& _rNameBase,
            const ::std::vector< OUString >& _rProhibitedNames );
        virtual ~NewDataTypeDialog();
        virtual void dispose() override;

        OUString GetName() const { return m_pName->GetText(); }

    private:
        DECL_LINK_TYPED( OnNameModified, Edit&, void );
    };


} // namespace pcr


#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_NEWDATATYPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
