import java.io.File;
import java.io.FileReader;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.util.Properties;

/** Load from and save options into a file.
*/
class Options
    extends Properties
{
    static public Options Instance ()
    {
        if (saOptions == null)
            saOptions = new Options ();
        return saOptions;
    }

    static public void SetString (String sName, String sValue)
    {
        Instance().setProperty (sName, sValue);
    }

    static public String GetString (String sName)
    {
        return Instance().getProperty (sName);
    }

    static public void SetBoolean (String sName, boolean bValue)
    {
        Instance().setProperty (sName, Boolean.toString(bValue));
    }

    static public boolean GetBoolean (String sName)
    {
        return Boolean.getBoolean(Instance().getProperty (sName));
    }

    static public void SetInteger (String sName, int nValue)
    {
        Instance().setProperty (sName, Integer.toString(nValue));
    }

    static public int GetInteger (String sName, int nDefault)
    {
        String sValue = Instance().getProperty (sName);
        if (sValue == null)
            return nDefault;
        else
            return Integer.parseInt (sValue);
    }

    public void Load (String sBaseName)
    {
        try
        {
            load (new FileInputStream (ProvideFile(sBaseName)));
        }
        catch (java.io.IOException e)
        {
            // Ignore a non-existing options file.
        }
    }

    public void Save (String sBaseName)
    {
        try
        {
            store (new FileOutputStream (ProvideFile(sBaseName)), null);
        }
        catch (java.io.IOException e)
        {
        }
    }

    private Options ()
    {
    }

    private File ProvideFile (String sBaseName)
    {
        return new File (
            System.getProperty ("user.home"),
            sBaseName);
    }

    static private Options saOptions = null;
}
