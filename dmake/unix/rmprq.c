/*************************************************************************
 *
 *  $RCSfile: rmprq.c,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 14:02:41 $
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
/*
--
-- SYNOPSIS
--      Remove prerequisites code.
--
-- DESCRIPTION
--  This code is different for DOS and for UNIX and parallel make
--  architectures since the parallel case requires the rm's to be
--  run in parallel, whereas DOS guarantees to run them sequentially.
--
-- AUTHOR
--      Dennis Vadura, dvadura@dmake.wticorp.com
--
-- WWW
--      http://dmake.wticorp.com/
--
-- COPYRIGHT
--      Copyright (c) 1996,1997 by WTI Corp.  All rights reserved.
--
--      This program is NOT free software; you can redistribute it and/or
--      modify it under the terms of the Software License Agreement Provided
--      in the file <distribution-root>/readme/license.txt.
--
-- LOG
--      Use cvs log to obtain detailed change logs.
*/

#include "extern.h"

PUBLIC void
Remove_prq( tcp )
CELLPTR tcp;
{
   static  LINKPTR rlp = NIL(LINK);
   static  int flag = 0;
   static  HASHPTR m_at, m_q, m_b, m_g, m_l, m_bb, m_up;
   char    *m_at_s, *m_g_s, *m_q_s, *m_b_s, *m_l_s, *m_bb_s, *m_up_s;
   LINKPTR tlp;

   tcp->ce_flag         &= ~(F_MADE|F_VISITED);
   tcp->ce_time          = 0L;

   for( tlp=rlp; tlp !=NIL(LINK); tlp=tlp->cl_next )
      if( (tlp->cl_prq->ce_flag & (F_VISITED|F_MADE)) != F_VISITED )
     break;

   if( tlp == NIL(LINK) ) {
      TALLOC(tlp, 1, LINK);
      TALLOC(tlp->cl_prq, 1, CELL);
      tlp->cl_next = rlp;
      rlp = tlp;
   }

   *tlp->cl_prq = *tcp;

   /* We save the dynamic macro values here, as it is possible that the
    * .REMOVE recipe is getting executed for a target while some other target
    * is in the middle of executing it's list of recipe lines, in this case
    * the values of $@ etc, must be preserved so that when we return to
    * complete the other recipe we must make certain that the values of it's
    * dynamic macros are unmodified. */

   if( !flag ) {
      /* Do the getting of the macros only once. */
      flag = 1;
      m_at = Get_name("@", Macs, TRUE);
      m_g  = Get_name(">", Macs, TRUE);
      m_q  = Get_name("?", Macs, TRUE);
      m_b  = Get_name("<", Macs, TRUE);
      m_l  = Get_name("&", Macs, TRUE);
      m_bb = Get_name("*", Macs, TRUE);
      m_up = Get_name("^", Macs, TRUE);
   }

   m_at_s = m_at->ht_value; m_at->ht_value = NIL(char);
   m_g_s  = m_g->ht_value;  m_g->ht_value  = NIL(char);
   m_q_s  = m_q->ht_value;  m_q->ht_value  = NIL(char);
   m_b_s  = m_b->ht_value;  m_b->ht_value  = NIL(char);
   m_l_s  = m_l->ht_value;  m_l->ht_value  = NIL(char);
   m_bb_s = m_bb->ht_value; m_bb->ht_value = NIL(char);
   m_up_s = m_up->ht_value; m_up->ht_value = NIL(char);

   Make( tlp->cl_prq, tcp );
   if( tlp->cl_prq->ce_dir ){
      FREE(tlp->cl_prq->ce_dir);
      tlp->cl_prq->ce_dir=NIL(char);
   }

   m_at->ht_value = m_at_s;
   m_g->ht_value  = m_g_s;
   m_q->ht_value  = m_q_s;
   m_b->ht_value  = m_b_s;
   m_l->ht_value  = m_l_s;
   m_bb->ht_value = m_bb_s;
   m_up->ht_value = m_up_s;
}
