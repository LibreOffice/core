/*************************************************************************
 *
 *  $RCSfile: inputopt.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:49 $
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

#ifndef SC_INPUTOPT_HXX
#define SC_INPUTOPT_HXX


#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

class SvStream;


class ScInputOptions
{
private:
    USHORT      nMoveDir;           // enum ScDirection
    BOOL        bMoveSelection;
    BOOL        bEnterEdit;
    BOOL        bExtendFormat;
    BOOL        bRangeFinder;
    BOOL        bExpandRefs;
    BOOL        bMarkHeader;
    BOOL        bUseTabCol;

public:
                ScInputOptions();
                ScInputOptions( const ScInputOptions& rCpy );
                ~ScInputOptions();

    void        SetDefaults();

    void        SetMoveDir(USHORT nNew)     { nMoveDir = nNew;       }
    USHORT      GetMoveDir() const          { return nMoveDir;       }
    void        SetMoveSelection(BOOL bSet) { bMoveSelection = bSet; }
    BOOL        GetMoveSelection() const    { return bMoveSelection; }
    void        SetEnterEdit(BOOL bSet)     { bEnterEdit = bSet;     }
    BOOL        GetEnterEdit() const        { return bEnterEdit;     }
    void        SetExtendFormat(BOOL bSet)  { bExtendFormat = bSet;  }
    BOOL        GetExtendFormat() const     { return bExtendFormat;  }
    void        SetRangeFinder(BOOL bSet)   { bRangeFinder = bSet;   }
    BOOL        GetRangeFinder() const      { return bRangeFinder;   }
    void        SetExpandRefs(BOOL bSet)    { bExpandRefs = bSet;    }
    BOOL        GetExpandRefs() const       { return bExpandRefs;    }
    void        SetMarkHeader(BOOL bSet)    { bMarkHeader = bSet;    }
    BOOL        GetMarkHeader() const       { return bMarkHeader;    }
    void        SetUseTabCol(BOOL bSet)     { bUseTabCol = bSet;     }
    BOOL        GetUseTabCol() const        { return bUseTabCol;     }

    const ScInputOptions&   operator=   ( const ScInputOptions& rOpt );
    friend SvStream&        operator>>  ( SvStream& rStream, ScInputOptions& rOpt );
    friend SvStream&        operator<<  ( SvStream& rStream, const ScInputOptions& rOpt );
};


//==================================================================
// CfgItem fuer Eingabe-Optionen
//==================================================================

class ScInputCfg : public ScInputOptions,
                  public utl::ConfigItem
{
    com::sun::star::uno::Sequence<rtl::OUString> GetPropertyNames();

public:
            ScInputCfg();

    void            SetOptions( const ScInputOptions& rNew );
    void            OptionsChanged();   // after direct access to SetOptions base class

    virtual void    Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames );
    virtual void    Commit();
};


#endif

