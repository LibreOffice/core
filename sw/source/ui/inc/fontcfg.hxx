/*************************************************************************
 *
 *  $RCSfile: fontcfg.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:40 $
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
#ifndef _FONTCFG_HXX
#define _FONTCFG_HXX

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#define FONT_STANDARD   0
#define FONT_OUTLINE    1
#define FONT_LIST       2
#define FONT_CAPTION    3
#define FONT_INDEX      4

class SwStdFontConfig : public utl::ConfigItem
{
    String      sFontStandard;
    String      sFontOutline ;
    String      sFontList    ;
    String      sFontCaption ;
    String      sFontIndex   ;

    com::sun::star::uno::Sequence<rtl::OUString>    GetPropertyNames();

public:
    SwStdFontConfig();
    ~SwStdFontConfig();

    virtual void    Commit();

    const String&   GetFontStandard() const {return sFontStandard   ;}
    const String&   GetFontOutline()  const {return sFontOutline ;}
    const String&   GetFontList   ()  const {return sFontList ;}
    const String&   GetFontCaption()  const {return sFontCaption ;}
    const String&   GetFontIndex  ()  const {return sFontIndex  ;}

    BOOL            IsFontDefault(USHORT nFontType) const;

    inline void     SetFontStandard(const String& rSet);
    inline void     SetFontOutline(const String& rSet);
    inline void     SetFontList   (const String& rSet);
    inline void     SetFontCaption(const String& rSet);
    inline void     SetFontIndex  (const String& rSet);

    static String   GetDefaultFor(USHORT nFontType);
};

inline void SwStdFontConfig::SetFontStandard   (const String& rSet)
{
    if( sFontStandard != rSet)
    {
        SetModified();
        sFontStandard = rSet;
    }
}
inline void SwStdFontConfig::SetFontOutline(const String& rSet)
{
    if( sFontOutline != rSet)
    {
        SetModified();
        sFontOutline  = rSet;
    }
}
inline void SwStdFontConfig::SetFontList      (const String& rSet)
{
    if(sFontList != rSet)
    {
        SetModified();
        sFontList = rSet;
    }
}

inline void SwStdFontConfig::SetFontCaption(const String& rSet)
{
    if(sFontCaption != rSet)
    {
        SetModified();
        sFontCaption  = rSet;
    }
}

inline void SwStdFontConfig::SetFontIndex  (const String& rSet)
{
    if(sFontIndex != rSet)
    {
        SetModified();
        sFontIndex = rSet;
    }
}
#endif


