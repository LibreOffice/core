/*************************************************************************
 *
 *  $RCSfile: unoedhlp.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: thb $ $Date: 2002-08-02 11:31:19 $
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

#ifndef _SVX_UNOEDHLP_HXX
#define _SVX_UNOEDHLP_HXX

#include <memory>

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _TEXTDATA_HXX
#include <svtools/textdata.hxx>
#endif
#ifndef _SFXHINT_HXX
#include <svtools/hint.hxx>
#endif

struct EENotify;
class EditEngine;

#define EDITSOURCE_HINT_PARASMOVED          20
#define EDITSOURCE_HINT_SELECTIONCHANGED    21

/** Extends TextHint by two additional parameters which are necessary
    for the EDITSOURCE_HINT_PARASMOVED hint. TextHint's value in this
    case denotes the destination position, the two parameters the
    start and the end of the moved paragraph range.
 */
class SvxEditSourceHint : public TextHint
{
private:
    ULONG   mnStart;
    ULONG   mnEnd;

public:
            TYPEINFO();
            SvxEditSourceHint( ULONG nId );
            SvxEditSourceHint( ULONG nId, ULONG nValue, ULONG nStart=0, ULONG nEnd=0 );

    ULONG   GetValue() const;
    ULONG   GetStartValue() const;
    ULONG   GetEndValue() const;
    void    SetValue( ULONG n );
    void    SetStartValue( ULONG n );
    void    SetEndValue( ULONG n );
};

/** Helper class for common functionality in edit sources
 */
class SvxEditSourceHelper
{
public:

    /** Translates EditEngine notifications into broadcastable hints

        @param aNotify
        Notification object send by the EditEngine.

        @return the translated hint
     */
    static ::std::auto_ptr<SfxHint> EENotification2Hint( EENotify* aNotify );

    /** Calculate attribute run for EditEngines

        Please note that the range returned is half-open: [nStartIndex,nEndIndex)

        @param nStartIndex
        Herein, the start index of the range of similar attributes is returned

        @param nEndIndex
        Herein, the end index (exclusive) of the range of similar attributes is returned

        @param rEE
        The EditEngine to query for attributes

        @param nPara
        The paragraph the following index value is to be interpreted in

        @param nIndex
        The character index from which the range of similar attributed characters is requested

        @return sal_True, if the range has been successfully determined
     */
    static sal_Bool GetAttributeRun( USHORT& nStartIndex, USHORT& nEndIndex, const EditEngine& rEE, USHORT nPara, USHORT nIndex );

};

#endif

