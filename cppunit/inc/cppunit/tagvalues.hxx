/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tagvalues.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lla $ $Date: 2008-02-27 16:09:11 $
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

#ifndef TAGVALUES_H
#define TAGVALUES_H

#include <cppunit/taghelper.hxx>

#define TAG_TYPE           (Tag(TAG_USER | (0x01L)))

#define TAG_RESULT (TAG_USER | 0x100)

#define RESULT_START       (Tag(TAG_RESULT | (0x02L)))
#define TAG_RESULT_PTR     (Tag(TAG_RESULT | (0x03L)))
#define TAG_TEST_PTR       (Tag(TAG_RESULT | (0x04L)))

#define RESULT_END         (Tag(TAG_RESULT | (0x05L)))

#define RESULT_ADD_FAILURE (Tag(TAG_RESULT | (0x06L)))
#define RESULT_ADD_ERROR   (Tag(TAG_RESULT | (0x07L)))

#define TAG_EXCEPTION      (Tag(TAG_RESULT | (0x08L)))

#define RESULT_ADD_INFO    (Tag(TAG_RESULT | (0x09L)))
#define TAG_INFO           (Tag(TAG_RESULT | (0x0aL)))

#define RESULT_ENTER_NODE  (Tag(TAG_RESULT | (0x0bL)))
#define RESULT_LEAVE_NODE  (Tag(TAG_RESULT | (0x0cL)))
#define TAG_NODENAME       (Tag(TAG_RESULT | (0x0dL)))

#define RESULT_SHOULD_STOP (Tag(TAG_RESULT | (0x0eL)))


#define TAG_SIGNAL              (TAG_USER | 0x200)
// #define SIGNAL_CHECK             (Tag(TAG_SIGNAL | (0x01L)))
// #define INIT_SIGNAL_HANDLING    (Tag(TAG_SIGNAL | (0x04L)))
// #define RELEASE_SIGNAL_HANDLING (Tag(TAG_SIGNAL | (0x05L)))

#define SIGNAL_START_TEST       (Tag(TAG_SIGNAL | (0x06L)))
#define SIGNAL_END_TEST         (Tag(TAG_SIGNAL | (0x07L)))

#define TAG_EXECUTION           (TAG_USER | 0x400)
#define EXECUTION_CHECK         (Tag(TAG_EXECUTION | (0x01)))
#define INIT_TEST               (Tag(TAG_EXECUTION | (0x02)))
#define RELEASE_TEST            (Tag(TAG_EXECUTION | (0x03)))
// #define EXECUTION_PUSH_NAME     (Tag(TAG_EXECUTION | (0x04)))
// #define EXECUTION_POP_NAME      (Tag(TAG_EXECUTION | (0x05)))

#define DO_NOT_EXECUTE 666
#define GO_EXECUTE 1

typedef sal_Int32 ReturnValue;

#endif
