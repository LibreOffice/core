/*************************************************************************
 *
 *  $RCSfile: fmtfld.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-01-05 15:50:21 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _FMTFLD_HXX
#define _FMTFLD_HXX

#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif
#ifndef _CALBCK_HXX //autogen
#include <calbck.hxx>
#endif

class SwField;
// ATT_FLD ***********************************

class SW_DLLPUBLIC SwFmtFld : public SfxPoolItem, public SwClient
{
    friend class SwTxtFld;
    friend void _InitCore();

    SwField *pField;
    SwTxtFld* pTxtAttr;     // mein TextAttribut

    SwFmtFld();             // das default-Attibut

    // geschuetzter CopyCtor
    // @@@ copy construction allowed, but copy assignment is not? @@@
    SwFmtFld& operator=(const SwFmtFld& rFld);

public:
    TYPEINFO();

    // single argument constructors shall be explicit.
    explicit SwFmtFld( const SwField &rFld );

    // @@@ copy construction allowed, but copy assignment is not? @@@
    SwFmtFld( const SwFmtFld& rAttr );

    virtual ~SwFmtFld();

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;

    virtual void Modify( SfxPoolItem* pOld, SfxPoolItem* pNew );
    virtual BOOL GetInfo( SfxPoolItem& rInfo ) const;

    const SwField *GetFld() const   { return pField; }
    SwField *GetFld()               { return pField; }

    // #111840#
    /**
       Sets current field.

       @param pField          new field

       @attention The current field will be destroyed before setting the new field.
     */
    void SetFld(SwField * pField);

    const SwTxtFld *GetTxtFld() const   { return pTxtAttr; }
    SwTxtFld *GetTxtFld()               { return pTxtAttr; }

    BOOL IsFldInDoc() const;
    BOOL IsProtect() const;
};


#endif

