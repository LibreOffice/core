/*************************************************************************
 *
 *  $RCSfile: attributes.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jb $ $Date: 2001-09-25 16:00:39 $
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

#ifndef CONFIGMGR_CONFIGURATION_ATTRIBUTES_HXX_
#define CONFIGMGR_CONFIGURATION_ATTRIBUTES_HXX_
namespace configmgr
{
    namespace configuration
    {
        /// holds attributes a node in the schema
        struct Attributes
        {
            bool bWritable      : 1;    // write-protected, if false
            bool bFinalized     : 1;    // write-protected, but not here

            bool bReplacing     : 1;    // node does not exist in the default layer
            bool bDefaultable   : 1;    // values only: a default value does exist

            bool bNullable      : 1;    // values only: can be NULL
            bool bLocalized     : 1;    // values only: value may depend on locale

            bool bNotified      : 1;    // can register a listener for changes to this node
            bool bConstrained   : 1;    // can register a veto listener to constrain changes to this node


            Attributes()
            : bWritable(true)
            , bNullable(true)
            , bNotified(true)
            , bConstrained(false)
            , bReplacing(false)
            , bLocalized(false)
            , bDefaultable(false)
            , bFinalized(false)
            {}
        };

    }
}

#endif
