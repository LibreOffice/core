/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xtextedt.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:44:21 $
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
#ifndef _XTEXTEDT_HXX
#define _XTEXTEDT_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _TEXTENG_HXX
#include <svtools/texteng.hxx>
#endif
#ifndef _TEXTVIEW_HXX
#include <svtools/textview.hxx>
#endif

#define XTEXTUNDO_REPLACEALL    (XTEXTUNDO_START+1)
#define XTEXTUNDO_INDENTBLOCK       122
#define XTEXTUNDO_UNINDENTBLOCK     123

namespace com {
namespace sun {
namespace star {
namespace util {
    struct SearchOptions;
}}}}

class SVT_DLLPUBLIC ExtTextEngine : public TextEngine
{
private:
    String              maGroupChars;

public:
                        ExtTextEngine();
                        ~ExtTextEngine();

    const String&       GetGroupChars() const { return maGroupChars; }
    void                SetGroupChars( const String& r ) { maGroupChars = r; }
    TextSelection       MatchGroup( const TextPaM& rCursor ) const;

    BOOL                Search( TextSelection& rSel, const ::com::sun::star::util::SearchOptions& rSearchOptions, BOOL bForward = TRUE );
};

class SVT_DLLPUBLIC ExtTextView : public TextView
{
protected:
    BOOL                ImpIndentBlock( BOOL bRight );

public:
                        ExtTextView( ExtTextEngine* pEng, Window* pWindow );
                        ~ExtTextView();

    BOOL                MatchGroup();

    BOOL                Search( const ::com::sun::star::util::SearchOptions& rSearchOptions, BOOL bForward );
    USHORT              Replace( const ::com::sun::star::util::SearchOptions& rSearchOptions, BOOL bAll, BOOL bForward );

    BOOL                IndentBlock();
    BOOL                UnindentBlock();
};

#endif // _XTEXTEDT_HXX
