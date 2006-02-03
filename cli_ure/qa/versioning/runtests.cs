
using System;
using System.Reflection;
using System.IO;

// __________  implementation  ____________________________________

/** Create and modify a spreadsheet document.
 */
namespace cliversion
{
public class RunTests
{

    public static int Main(String[] args)
    {
//        System.Diagnostics.Debugger.Launch();
        //get the path to the directory
        string sLocation = Assembly.GetExecutingAssembly().Location;
        sLocation = sLocation.Substring(0, sLocation.LastIndexOf('\\'));
        // Create a reference to the current directory.
        DirectoryInfo di = new DirectoryInfo(sLocation);
        // Create an array representing the files in the current directory.
        FileInfo[] fi = di.GetFiles();

        //For every found dll try to figure out if it contains a
        //cliversion.Version class
		foreach (FileInfo fiTemp in fi)
		{
			if (fiTemp.Extension != ".dll")
				continue;

            Assembly ass = null;
            Object objVersion = null;
			try
			{
                string sName = fiTemp.Name.Substring(0, fiTemp.Name.LastIndexOf(".dll"));
				ass = Assembly.Load(sName);
			}
			catch (BadImageFormatException)
			{
				continue;
			}
			catch (Exception e)
			{
				Console.WriteLine("#Unexpected Exception");
				Console.WriteLine(e.Message);
                return -1;
			}

            //Assembly is loaded, instantiate cliversion.Version
			try
			{
				//This runs the test
				objVersion = ass.CreateInstance("cliversion.Version");
				if (objVersion == null)
					continue;
				Console.WriteLine("#Tested successfully " + fiTemp.Name);
				//Starting the office the second time may fail without this pause
				System.Threading.Thread.Sleep(2000);
			}
			catch (Exception e)
			{
				TargetInvocationException te = e as TargetInvocationException;
				if (te != null)
				{
					FileNotFoundException fe = e.InnerException as FileNotFoundException;
					if (fe != null)
					{
						Console.WriteLine(fiTemp.Name + " did not find " + fe.FileName +
							". Maybe the " + fe.FileName + " is not installed or does not match the referenced version."  +
							"Original message: " + fe.Message + "\n\n FusionLog: \n" + fe.FusionLog );
						return -1;
					}
					FileLoadException fl = e.InnerException as FileLoadException;
					if (fl != null)
					{
						Console.WriteLine(fiTemp.Name + " could not load " + fl.FileName +
							". Maybe the version of " + fl.FileName + " does not match the referenced version. " +
							"Original message: " + fl.Message + "\n\n FusionLog: \n" + fl.FusionLog );
						return -1;
					}
				}
				Console.WriteLine("#Unexpected Exception");
				Console.WriteLine(e.Message);
				return -1;
			}
		}
        //For some unknown reason this program hangs sometimes when started from java. This is
        //a workaround that makes the problem disappear.
        System.Threading.Thread.Sleep(1000);
        return 0;
    }
}
}
