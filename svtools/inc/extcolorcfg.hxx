/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: extcolorcfg.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-20 19:16:18 $
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
#ifndef INCLUDED_SVTOOLS_EXTCOLORCFG_HXX
#define INCLUDED_SVTOOLS_EXTCOLORCFG_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _SFXBRDCST_HXX
#include <svtools/brdcst.hxx>
#endif
#ifndef _SFXLSTNER_HXX
#include <svtools/lstner.hxx>
#endif

//-----------------------------------------------------------------------------
namespace svtools{
/* -----------------------------22.03.2002 15:36------------------------------

 ---------------------------------------------------------------------------*/
class ExtendedColorConfig_Impl;
class ExtendedColorConfigValue
{
    ::rtl::OUString m_sName;
    ::rtl::OUString m_sDisplayName;
    sal_Int32       m_nColor;
    sal_Int32       m_nDefaultColor;
public:
    ExtendedColorConfigValue() : m_nColor(0),m_nDefaultColor(0){}
    ExtendedColorConfigValue(const ::rtl::OUString& _sName
                            ,const ::rtl::OUString& _sDisplayName
                            ,sal_Int32      _nColor
                            ,sal_Int32      _nDefaultColor)
    : m_sName(_sName)
    ,m_sDisplayName(_sDisplayName)
    ,m_nColor(_nColor)
    ,m_nDefaultColor(_nDefaultColor)
    {}

    inline ::rtl::OUString getName()         const { return m_sName; }
    inline ::rtl::OUString getDisplayName()  const { return m_sDisplayName; }
    inline sal_Int32       getColor()        const { return m_nColor; }
    inline sal_Int32       getDefaultColor() const { return m_nDefaultColor; }

    inline void setColor(sal_Int32 _nColor) { m_nColor = _nColor; }

    sal_Bool operator !=(const ExtendedColorConfigValue& rCmp) const
        { return m_nColor != rCmp.m_nColor;}
};
/* -----------------------------22.03.2002 15:36------------------------------

 ---------------------------------------------------------------------------*/
class SVT_DLLPUBLIC ExtendedColorConfig : public SfxBroadcaster, public SfxListener
{
    friend class ExtendedColorConfig_Impl;
private:
    static ExtendedColorConfig_Impl* m_pImpl;
public:
    ExtendedColorConfig();
    ~ExtendedColorConfig();

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    // get the configured value
    ExtendedColorConfigValue        GetColorValue(const ::rtl::OUString& _sComponentName,const ::rtl::OUString& _sName)const;
    sal_Int32                       GetComponentCount() const;
    ::rtl::OUString                 GetComponentName(sal_uInt32 _nPos) const;
    ::rtl::OUString                 GetComponentDisplayName(const ::rtl::OUString& _sComponentName) const;
    sal_Int32                       GetComponentColorCount(const ::rtl::OUString& _sName) const;
    ExtendedColorConfigValue        GetComponentColorConfigValue(const ::rtl::OUString& _sComponentName,sal_uInt32 _nPos) const;
};
/* -----------------------------22.03.2002 15:31------------------------------

 ---------------------------------------------------------------------------*/
class SVT_DLLPUBLIC EditableExtendedColorConfig
{
    ExtendedColorConfig_Impl*   m_pImpl;
    sal_Bool            m_bModified;
public:
    EditableExtendedColorConfig();
    ~EditableExtendedColorConfig();

    ::com::sun::star::uno::Sequence< ::rtl::OUString >  GetSchemeNames() const;
    void                                                DeleteScheme(const ::rtl::OUString& rScheme );
    void                                                AddScheme(const ::rtl::OUString& rScheme );
    sal_Bool                                            LoadScheme(const ::rtl::OUString& rScheme );
    const ::rtl::OUString&                              GetCurrentSchemeName()const;
    void                        SetCurrentSchemeName(const ::rtl::OUString& rScheme);

    ExtendedColorConfigValue    GetColorValue(const ::rtl::OUString& _sComponentName,const ::rtl::OUString& _sName)const;
    sal_Int32                   GetComponentCount() const;
    ::rtl::OUString             GetComponentName(sal_uInt32 _nPos) const;
    ::rtl::OUString             GetComponentDisplayName(const ::rtl::OUString& _sComponentName) const;
    sal_Int32                   GetComponentColorCount(const ::rtl::OUString& _sName) const;
    ExtendedColorConfigValue    GetComponentColorConfigValue(const ::rtl::OUString& _sName,sal_uInt32 _nPos) const;
    void                        SetColorValue(const ::rtl::OUString& _sComponentName, const ExtendedColorConfigValue& rValue);
    void                        SetModified();
    void                        ClearModified(){m_bModified = sal_False;}
    sal_Bool                    IsModified()const{return m_bModified;}
    void                        Commit();

    void                        DisableBroadcast();
    void                        EnableBroadcast();
};
}//namespace svtools
#endif

