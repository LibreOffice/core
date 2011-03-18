public class NewsGroup
{

    private String hostname = "";
    private String newsgroupName = "";

    public NewsGroup( String host, String group )
    {
        hostname = host;
        newsgroupName = group;
    }

    public String getHostName()
    {
        return hostname;
    }

    public String getNewsgroupName()
    {
        return newsgroupName;
    }

}
