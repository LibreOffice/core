/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SpellAttrib.hxx,v $
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
#ifndef _SVX_SPELL_ATTRIB
#define _SVX_SPELL_ATTRIB

#include <svtools/txtattr.hxx>
#include <i18npool/lang.h>
#include <com/sun/star/uno/Reference.h>
#include <tools/color.hxx>
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
