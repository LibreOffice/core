/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cfgdesc.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:02:46 $
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
#ifndef _CFGDESC_HXX
#define _CFGDESC_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

class SwCfgDesc
{
protected:
    String aFuncText;
    String aLibName;
    USHORT nEvent;

    SwCfgDesc(const String &rFuncName, USHORT nId);
    SwCfgDesc(const String &rLib,const String &rMacro);
    ~SwCfgDesc();

public:
    void SetEvent(USHORT nE);               // inline
    void SetFuncText(const String &rStr);   // inline
    void SetMacro(const String &rLib, const String &rMac);
    BOOL IsMacro() const;
    USHORT GetEvent() const;                // inline
    String GetFuncText() const;             // inline
    String GetMacro() const;                // inline
    String GetLib() const;              // inline
};

// INLINE METHODE --------------------------------------------------------
inline void SwCfgDesc::SetEvent(USHORT nE) {
    nEvent = nE;
}

inline void SwCfgDesc::SetFuncText(const String &rStr) {
    aFuncText = rStr;
}

inline  USHORT SwCfgDesc::GetEvent() const { return nEvent; }
inline  String SwCfgDesc::GetFuncText() const { return aFuncText; }
inline  String SwCfgDesc::GetMacro() const { return aFuncText; }
inline  String SwCfgDesc::GetLib() const { return aLibName; }


#endif
