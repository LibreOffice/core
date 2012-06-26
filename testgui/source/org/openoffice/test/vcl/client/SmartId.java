/************************************************************************
 *
 * Licensed Materials - Property of IBM.
 * (C) Copyright IBM Corporation 2003, 2012.  All Rights Reserved.
 * U.S. Government Users Restricted Rights:
 * Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 ************************************************************************/

package org.openoffice.test.vcl.client;


/**
 * ID of GUI controls may have two types: number or string.
 * From AOO3.4, all IDs should be string.
 */
public class SmartId {

    private long id = 0;

    private String sid = null;

    public SmartId(long id) {
        super();
        this.id = id;
    }

    public SmartId(String sid) {
        super();
        this.sid = sid;
    }

    public long getId() {
        return id;
    }

    public String getSid() {
        return sid;
    }

    public String toString() {
        if (sid == null)
            return Long.toString(id);
        else
            return sid;
    }

    public int hashCode() {
        if (sid == null)
            return new Long(id).hashCode();
        return sid.hashCode();
    }

    public boolean equals (Object o) {
        if (!(o instanceof SmartId))
            return false;
        SmartId id2 = (SmartId) o;
        return id2.id == this.id && ((this.sid == null && id2.sid == null) || (this.sid != null && this.sid.equals(id2.sid)));
    }
}
