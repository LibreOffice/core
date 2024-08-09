using NUnit.Framework;

using com.sun.star.uno;

[TestFixture]
public class AnyTests
{
    [Test]
    [Description("Tests if Any.VOID has a null value and type as void")]
    public void VOID_HasNullValueAndVoidType()
    {
        Assert.That(Any.VOID, Is.Not.Null);
        Assert.That(Any.VOID.Value, Is.Null);
        Assert.That(Any.VOID.Type, Is.EqualTo(typeof(void)));
    }

    [Test]
    [Description("Tests if the Any constructor throws on invalid args")]
    public void ctor_RejectsInvalidParams()
    {
        Assert.That(() => new Any(null, 10), Throws.ArgumentNullException);
        Assert.That(() => new Any(typeof(Any), Any.VOID), Throws.ArgumentException);
        Assert.That(() => new Any(typeof(int), null), Throws.ArgumentException);
    }

    [Test]
    [Description("Tests if Any.with throws on invalid args")]
    public void with_RejectsInvalidParams()
    {
        Assert.That(() => Any.with<Any>(Any.VOID), Throws.ArgumentException);
        Assert.That(() => Any.with<int>(null), Throws.ArgumentException);
    }

    [Test]
    [Description("Tests if Any.equals and Any.operator== returns true for identical values")]
    public void equals_ReturnsTrueForSame()
    {
        Assert.That(Any.VOID == new Any(typeof(void), null), Is.True);
        Assert.That(Any.with<int>(10).equals(new Any(typeof(int), 10)), Is.True);
        Assert.That(new Any(typeof(bool), false).Equals(new Any(false)), Is.True);
    }

    [Test]
    [Description("Tests if Any.equals and Any.operator== returns false for different values")]
    public void equals_ReturnsTrueForDifferent()
    {
        Assert.That(Any.VOID == Any.with<int>(10), Is.False);
        Assert.That(Any.VOID != new Any(10), Is.True);
        Assert.That(Any.with<int>(10).equals(new Any(20)), Is.False);
        Assert.That(new Any(typeof(bool), true).Equals(Any.with<int>(1)), Is.False);
    }

    [Test]
    [Description("Tests if Any.hasValue() returns false for Any.VOID and equivalent values")]
    public void hasValue_ReturnsFalseForVOIDValues()
    {
        Assert.That(Any.VOID.hasValue, Is.False);
        Assert.That(new Any(null).hasValue, Is.False);
        Assert.That(Any.with<int>(10).hasValue, Is.True);
        Assert.That(new Any(typeof(string), "hello").hasValue, Is.True);
    }

    [Test]
    [Description("Tests if Any.setValue(type, value) method updates type and value correctly")]
    public void setValue_UpdatesTypeAndValue()
    {
        Any any = new Any(typeof(int), 10);
        Assert.That(any.Type, Is.EqualTo(typeof(int)));
        Assert.That(any.Value, Is.EqualTo(10));
        any.setValue(typeof(string), "hello");
        Assert.That(any.Type, Is.EqualTo(typeof(string)));
        Assert.That(any.Value, Is.EqualTo("hello"));
        any.setValue(3.14);
        Assert.That(any.Type, Is.EqualTo(typeof(double)));
        Assert.That(any.Value, Is.EqualTo(3.14));
        any.setValue<bool>(false);
        Assert.That(any.Type, Is.EqualTo(typeof(bool)));
        Assert.That(any.Value, Is.EqualTo(false));
    }

    [Test]
    [Description("Tests if Any.setValue(type, value) method throws on invalid args")]
    public void setValue_RejectsInvalidParams()
    {
        Any any = new Any(10);
        Assert.That(() => any.setValue(null, 10), Throws.ArgumentNullException);
        Assert.That(() => any.setValue(Any.VOID), Throws.ArgumentException);
        Assert.That(() => any.setValue(typeof(int), null), Throws.ArgumentException);
    }

    [Test]
    [Description("Tests if Any.GetHashCode() returns same hash for identical values")]
    public void GetHashCode_ReturnsTrueForSame()
    {
        Assert.That(Any.VOID.GetHashCode() == Any.VOID.GetHashCode(), Is.True);
        Assert.That(new Any(10).GetHashCode() == Any.with<int>(10).GetHashCode(), Is.True);
    }

    [Test]
    [Description("Tests if Any.GetHashCode() returns different hash for different values")]
    public void GetHashCode_ReturnsTrueForDifferent()
    {
        Assert.That(Any.VOID.GetHashCode() == Any.with<int>(10).GetHashCode(), Is.False);
    }
}
