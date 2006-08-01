/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DiffAlgorithm.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-01 13:33:53 $
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

/**
 * This is the difference algorithm interface.  It is an interface so
 * that different algorithms may be plugged-in to actually compute
 * the differences.
 *
 * NOTE: this code may not be thread safe.
 */
public interface DiffAlgorithm {

    /**
     *  Returns a <code>Difference</code> array.  This method finds out
     *  the difference between two sequences.
     *
     *  @param  orgSeq  The original sequence of object.
     *  @param  modSeq  The modified (or changed) sequence to
     *                  compare against with the origial.
     *
     *  @return  A <code>Difference</code> array.
     */
    public Difference[] computeDiffs(Iterator orgSeq, Iterator modSeq);
}

