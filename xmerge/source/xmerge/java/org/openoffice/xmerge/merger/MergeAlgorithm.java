/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MergeAlgorithm.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 13:34:26 $
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

package org.openoffice.xmerge.merger;

import org.openoffice.xmerge.MergeException;
import org.openoffice.xmerge.merger.Iterator;
import org.openoffice.xmerge.merger.Difference;

/**
 *  This is the <code>MergeAlgorithm</code> interface. It is an
 *  interface so that different merge algorithms may be plugged-in
 *  to actually merge the diffs back to an original document.
 *
 *  @author smak
 */
public interface MergeAlgorithm {

    /**
     *  This method is to merge the difference to an <code>Iterator</code>.
     *  The original <code>Iterator</code> will be modified after the call.
     *
     *  @param  objSeq       The original sequence which the difference
     *                       will be applied.  It will be modified.
     *  @param  modSeq       The modified sequence where the difference
     *                       content will be extracted.
     *  @param  differences  The <code>Difference</code> array.
     *
     *  @return  An <code>Iterator</code> which is the modified original
     *           <code>Iterator</code> Sequence.  Same as the first parameter.
     *
     *  @throws  MergeException  If an error occurs during the merge.
     */
    public void applyDifference(Iterator orgSeq, Iterator modSeq,
                                Difference[] differences) throws MergeException;
}

