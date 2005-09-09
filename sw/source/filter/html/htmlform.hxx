/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: htmlform.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:43:54 $
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

#ifndef _HTMLFORM_HXX
#define _HTMLFORM_HXX


enum HTMLEventType
{
    HTML_ET_ONSUBMITFORM,   HTML_ET_ONRESETFORM,
    HTML_ET_ONGETFOCUS,     HTML_ET_ONLOSEFOCUS,
    HTML_ET_ONCLICK,        HTML_ET_ONCLICK_ITEM,
    HTML_ET_ONCHANGE,       HTML_ET_ONSELECT,
    HTML_ET_END
};

extern HTMLEventType __FAR_DATA aEventTypeTable[];
extern const sal_Char * __FAR_DATA aEventListenerTable[];
extern const sal_Char * __FAR_DATA aEventMethodTable[];
extern const sal_Char * __FAR_DATA aEventSDOptionTable[];
extern const sal_Char * __FAR_DATA aEventOptionTable[];




#endif


