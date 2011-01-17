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

#ifndef SD_OPTDLG_HXX
#define SD_OPTDLG_HXX

#include <sfx2/tabdlg.hxx>
#include "pres.hxx"

class SfxItemSet;


class SdOptionsDlg : public SfxTabDialog
{
private:
    DocumentType    meDocType;

public:
                SdOptionsDlg( Window* pParent, const SfxItemSet& rInAttrs,
                              DocumentType eDocType );
                ~SdOptionsDlg();

protected:

    virtual void        PageCreated( sal_uInt16 nId, SfxTabPage &rPage );
//  virtual SfxItemSet* CreateInputItemSet( sal_uInt16 nPageId );
};



#endif // SD_OPTDLG_HXX


