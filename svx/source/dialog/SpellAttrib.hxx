/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SpellAttrib.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2006-04-07 13:58:36 $
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
#ifndef _SVX_SPELL_ATTRIB
#define _SVX_SPELL_ATTRIB

#ifndef _TXTATTR_HXX
#include <svtools/txtattr.hxx>
#endif
#ifndef INCLUDED_I18NPOOL_LANG_H
#include <i18npool/lang.h>
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
