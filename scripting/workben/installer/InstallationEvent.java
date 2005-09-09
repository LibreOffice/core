package installer;

public class InstallationEvent
{
    private Object source;
    private String message;
    InstallationEvent(Object source, String message)
    {
        this.source = source;
        this.message = message;
    }

    public Object getSource()
    {
        return source;
    }

    public String getMessage()
    {
        return message;
    }
}
