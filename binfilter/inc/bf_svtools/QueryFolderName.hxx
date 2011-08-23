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
#ifndef SVT_QUERYFOLDER_HXX
#define SVT_QUERYFOLDER_HXX

#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <vcl/dialog.hxx>

//-------------------------------------------------------------------------
namespace binfilter
{

//-------------------------------------------------------------------------
// QueryFolderNameDialog
//-------------------------------------------------------------------------

class QueryFolderNameDialog : public ModalDialog
{
private:
    FixedText		aNameText;
    Edit			aNameEdit;
    FixedLine		aNameLine;
    OKButton		aOKBtn;
    CancelButton	aCancelBtn;

    DECL_LINK( OKHdl, Button * );
    DECL_LINK( NameHdl, Edit * );

public:
                    QueryFolderNameDialog(	Window* _pParent,
                                            const String& rTitle,
                                            const String& rDefaultText,
                                            String* pGroupName = NULL );
    String			GetName() const { return aNameEdit.GetText(); }
};

}
#endif // SVT_QUERYFOLDER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
