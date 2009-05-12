/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: bibmod.hxx,v $
 * $Revision: 1.5 $
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

#ifndef BIBMOD_HXX
#define BIBMOD_HXX


class ResMgr;
class BibDataManager;
class BibConfig;

#define BIBLIOGRAPHY_INI_GROUP      "Bibliography"
#define BIBLIOGRAPHY_INI_CUR_NAME   "BibliographyCurrent"
#define BIBLIOGRAPHY_INI_DB_ENTRY   "Bibliography"
#define BIBLIOGRAPHY_INI_MAPPING    "BibliographyFields"
#define BIBLIOGRAPHY_TYPE_NAMES     "BibliographyTypeNames"

class BibModul
{
    private:
        ResMgr*                 pResMgr;
        static BibConfig*       pBibConfig;

    public:
                                BibModul();
                                ~BibModul();

        ResMgr*                 GetResMgr(){return pResMgr;}
        static BibConfig*       GetConfig();

        BibDataManager*         createDataManager();

};

typedef BibModul*       PtrBibModul;
typedef PtrBibModul*    HdlBibModul;

HdlBibModul     OpenBibModul();
void            CloseBibModul(HdlBibModul ppBibModul);

#endif
