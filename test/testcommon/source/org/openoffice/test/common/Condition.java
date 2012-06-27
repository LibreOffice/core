/************************************************************************
 *
 * Licensed Materials - Property of IBM.
 * (C) Copyright IBM Corporation 2003, 2012.  All Rights Reserved.
 * U.S. Government Users Restricted Rights:
 * Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 ************************************************************************/

package org.openoffice.test.common;


public abstract class Condition {
    /**
     *
     * @return true, if the condition is true. false, if it's not true.
     */
    public abstract boolean value();


    public boolean test(double iTimeout, double interval) {
        long startTime = System.currentTimeMillis();
        while (System.currentTimeMillis() - startTime < iTimeout * 1000) {
            if (value())
                return true;
            try {
                Thread.sleep((long) (interval * 1000));
            } catch (InterruptedException e) {
            }
        }

        return value();
    }

    public void waitForTrue(String message, double iTimeout, double interval) {
        if (!test(iTimeout, interval)) {
            throw new RuntimeException(message);
        }
    }
}
