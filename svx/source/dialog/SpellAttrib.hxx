/*************************************************************************
 *
 *  $RCSfile: SpellAttrib.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-09-17 14:12:58 $
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
#ifndef _SVX_SPELL_ATTRIB
#define _SVX_SPELL_ATTRIB

#ifndef _TXTATTR_HXX
#include <svtools/txtattr.hxx>
#endif
#ifndef _LANG_HXX
#include <tools/lang.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif
namespace com{ namespace sun{ namespace star{ namespace linguistic2{
    class XSpellAlternatives;
}}}}

#define TEXTATTR_SPELL_ERROR            (TEXTATTR_USER_START + 1)
#define TEXTATTR_SPELL_LANGUAGE         (TEXTATTR_USER_START + 2)
#define TEXTATTR_SPELL_BACKGROUND       (TEXTATTR_USER_START + 3)

namespace svx{
/* -----------------10.09.2003 14:23-----------------

 --------------------------------------------------*/
class SpellErrorAttrib : public TextAttrib
{
    com::sun::star::uno::Reference<com::sun::star::linguistic2::XSpellAlternatives> m_xAlternatives;

                            //not accessible
                            SpellErrorAttrib();
public:
                            SpellErrorAttrib(com::sun::star::uno::Reference<com::sun::star::linguistic2::XSpellAlternatives> xAlternatives);
                            SpellErrorAttrib( const SpellErrorAttrib& rAttr );
                            ~SpellErrorAttrib();

    com::sun::star::uno::Reference<com::sun::star::linguistic2::XSpellAlternatives>
                            GetAlternatives() const { return m_xAlternatives;}
    void                    SetAlternatives(com::sun::star::uno::Reference<com::sun::star::linguistic2::XSpellAlternatives> xAlt)
                                {m_xAlternatives = xAlt;}

    virtual void            SetFont( Font& rFont ) const;
    virtual TextAttrib*     Clone() const;
    virtual int             operator==( const TextAttrib& rAttr ) const;
};
/* -----------------10.09.2003 14:23-----------------

 --------------------------------------------------*/
class SpellLanguageAttrib : public TextAttrib
{
    LanguageType m_eLanguage;

                            //not accessible
                            SpellLanguageAttrib();

public:
                            SpellLanguageAttrib(LanguageType eLanguage);
                            SpellLanguageAttrib( const SpellLanguageAttrib& rAttr );
                            ~SpellLanguageAttrib();

    LanguageType            GetLanguage() const {return m_eLanguage;}
    void                    SetLanguage(LanguageType eLang)
                                        {m_eLanguage = eLang;}


    virtual void            SetFont( Font& rFont ) const;
    virtual TextAttrib*     Clone() const;
    virtual int             operator==( const TextAttrib& rAttr ) const;
};
/* -----------------31.10.2003 16:01-----------------

 --------------------------------------------------*/
class SpellBackgroundAttrib : public TextAttrib
{
    Color   m_aBackgroundColor;

                            //not accessible
                            SpellBackgroundAttrib();

public:
                            SpellBackgroundAttrib(const Color& rCol);
                            SpellBackgroundAttrib( const SpellBackgroundAttrib& rAttr );
                            ~SpellBackgroundAttrib();

    const   Color&          GetColor() const { return m_aBackgroundColor;}
    void                    SetColor( const Color& rNewCol ){m_aBackgroundColor = rNewCol;}


    virtual void            SetFont( Font& rFont ) const;
    virtual TextAttrib*     Clone() const;
    virtual int             operator==( const TextAttrib& rAttr ) const;
};
}//namespace svx
#endif
