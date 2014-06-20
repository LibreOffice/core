package org.apache.openoffice.ooxml.framework.part;

/** Operations around part names.
 */
public class PartName
    implements Comparable<PartName>
{
    public PartName (final String sPath)
    {
        if ( ! (sPath.isEmpty() || sPath.startsWith("/")))
        {
            assert(sPath.isEmpty() || sPath.startsWith("/"));
        }
        assert(sPath.indexOf('\\') == -1);

        msPath = sPath;
    }




    public PartName (
        final String sPath,
        final PartName aParentName,
        final String sMode)
    {
        switch(sMode)
        {
            case "External":
                msPath = sPath;
                break;

            case "Internal":
                msPath = Cleanup(aParentName.GetPathname() + "/" + sPath);
                break;

            default:
                throw new RuntimeException();
        }
    }




    public PartName getRelationshipsPartName ()
    {
        return new PartName(GetPathname() + "/_rels/" + GetBasename() + ".rels");
    }




    private String GetPathname ()
    {
        if (msPath.isEmpty())
            return "";
        else
        {
            final int nPathnameEnd = msPath.lastIndexOf('/');
            assert(nPathnameEnd>=0);
            return msPath.substring(0, nPathnameEnd);
        }
    }




    public String GetBasename ()
    {
        if (msPath.isEmpty())
            return "";
        else
        {
            final int nBasenameStart = msPath.lastIndexOf('/');
            assert(nBasenameStart>=0);
            return msPath.substring(nBasenameStart+1);
        }
    }




    public String GetExtension ()
    {
        final int nExtensionStart = msPath.lastIndexOf('.');
        if (nExtensionStart < 0)
            return null;
        else
            return msPath.substring(nExtensionStart+1);
    }




    public String GetFullname()
    {
        return msPath;
    }




    @Override
    public int compareTo (final PartName aOther)
    {
        return msPath.compareTo(aOther.msPath);
    }




    private String Cleanup (final String sName)
    {
        return sName.replaceAll("/[^/]+/\\.\\./", "/");
    }




    @Override
    public String toString ()
    {
        return msPath;
    }




    private final String msPath;
}
