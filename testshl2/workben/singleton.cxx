class OSingleton
{
public:
    static OSingleton * GetSingleton();
    virtual ~OSingleton(){}
protected :
    OSingleton(){}
    static OSingleton * m_pSingleton;
};

OSingleton * OSingleton::m_pSingleton = NULL;
OSingleton * OSingleton::GetSingleton()
{
    if(m_pSingleton  == NULL )
    {
        m_pSingleton = new OSingleton;
    }
    return m_pSingleton;
}
