/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bibmod.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 19:14:12 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
