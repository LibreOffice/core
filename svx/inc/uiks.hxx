/*************************************************************************
 *
 *  $RCSfile: uiks.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:03 $
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
#ifndef _SVX_UIKS_HXX
#define _SVX_UIKS_HXX

#ifndef _USR_UIKS_HXX
#include <usr/uiks.hxx>
#endif

//  DBENGINE

#define UIK_XDATABASEENGINE                             UIK_DATABASE(00)
#define UIK_XDATABASEFAVORITES                          UIK_DATABASE(01)
#define UIK_XDATABASE                                   UIK_DATABASE(02)
#define UIK_XDATABASECONNECTION                         UIK_DATABASE(03)
#define UIK_XTRANSACTIONSUPPORT                         UIK_DATABASE(04)
#define UIK_XDATABASECURSOR                             UIK_DATABASE(05)
#define UIK_XDATABASETABLE                              UIK_DATABASE(06)
#define UIK_XDATABASETABLES                             UIK_DATABASE(07)
#define UIK_XDATABASEQUERY                              UIK_DATABASE(08)
#define UIK_XDATABASEQUERIES                            UIK_DATABASE(09)
#define UIK_XDATABASERELATION                           UIK_DATABASE(0a)
#define UIK_XDATABASERELATIONS                          UIK_DATABASE(0b)
#define UIK_XDATABASEFIELD                              UIK_DATABASE(0c)
#define UIK_XDATABASEFIELDS                             UIK_DATABASE(0d)
#define UIK_XDATABASEINDEX                              UIK_DATABASE(0e)
#define UIK_XDATABASEINDEXES                            UIK_DATABASE(0f)
#define UIK_XDATABASEDOCUMENT                           UIK_DATABASE(10)
#define UIK_XDATABASEDOCUMENTS                          UIK_DATABASE(11)
#define UIK_XDATABASEWORKSPACE                          UIK_DATABASE(12)
#define UIK_XDATABASEWORKSPACES                         UIK_DATABASE(13)
#define UIK_XDATABASEITERATOR                           UIK_DATABASE(14)
#define UIK_XPREPAREDDATABASECURSOR                     UIK_DATABASE(15)


//  DBENGINE

//  FORMS

#define UIK_XFORM                                       UIK_FORMS(01)
#define UIK_XFORMS                                      UIK_FORMS(02)
#define UIK_XFORMCONTROL                                UIK_FORMS(03)

#define UIK_XHTMLFORM                                   UIK_FORMS(05)
#define UIK_XHTMLFORMLISTENER                           UIK_FORMS(06)
#define UIK_XDATABASEFORM                               UIK_FORMS(07)
#define UIK_XBOUNDCONTROL                               UIK_FORMS(08)
#define UIK_XINSERTRECORDLISTENER                       UIK_FORMS(09)
#define UIK_XUPDATERECORDLISTENER                       UIK_FORMS(0a)
#define UIK_XDESTROYRECORDLISTENER                      UIK_FORMS(0b)
#define UIK_XCURRENTRECORDLISTENER                      UIK_FORMS(0c)
#define UIK_XBOUNDCONTROLLISTENER                       UIK_FORMS(0d)
#define UIK_XLOADLISTENER                               UIK_FORMS(0e)
#define UIK_XERRORLISTENER                              UIK_FORMS(0f)

#define UIK_XFORMCONTROLFACTORY                         UIK_FORMS(10)
#define UIK_XFORMCONTROLLER                             UIK_FORMS(11)
#define UIK_XFORMCONTROLLERLISTENER                     UIK_FORMS(12)

// FORMS



#endif

