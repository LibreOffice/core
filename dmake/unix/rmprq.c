/* RCS  $Id: rmprq.c,v 1.1.1.1 2000-09-22 15:33:33 hr Exp $
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
   static  flag = 0;
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
