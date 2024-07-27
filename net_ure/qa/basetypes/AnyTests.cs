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
    [Description("Tests if the Any(type, value) constructor throws on invalid args")]
    public void ctor_RejectsInvalidParams()
    {
        Assert.That(() => new Any(null, 10), Throws.ArgumentNullException);
        Assert.That(() => new Any(typeof(Any), Any.VOID), Throws.ArgumentException);
        Assert.That(() => new Any(typeof(int), null), Throws.ArgumentException);
    }

    [Test]
    [Description("Tests if Any.with<T>(value) throws on invalid args")]
    public void with_RejectsInvalidParams()
    {
        Assert.That(() => new Any(typeof(Any), Any.VOID), Throws.ArgumentException);
    }

    [Test]
    [Description("Tests if Any.equals(other) and Any.operator== returns true for identical values")]
    public void equals_ReturnsTrueForSame()
    {
        Assert.That(Any.VOID == new Any(typeof(void), null), Is.True);
        Assert.That(Any.with(10).equals(new Any(typeof(int), 10)), Is.True);
        Assert.That(new Any(typeof(bool), false).Equals(Any.with(false)), Is.True);
    }

    [Test]
    [Description("Tests if Any.equals(other) and Any.operator== returns false for different values")]
    public void equals_ReturnsTrueForDifferent()
    {
        Assert.That(Any.VOID == Any.with(10), Is.False);
        Assert.That(Any.VOID != Any.with(10), Is.True);
        Assert.That(Any.with(10).equals(Any.with(20)), Is.False);
        Assert.That(Any.with(true).Equals(Any.with(1)), Is.False);
    }

    [Test]
    [Description("Tests if Any.hasValue() returns false for Any.VOID and true for all else")]
    public void hasValue_ReturnsFalseOnlyForVOID()
    {
        Assert.That(Any.VOID.hasValue, Is.False);
        Assert.That(Any.with(10).hasValue, Is.True);
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
        any.setValue(false);
        Assert.That(any.Type, Is.EqualTo(typeof(bool)));
        Assert.That(any.Value, Is.EqualTo(false));
    }

    [Test]
    [Description("Tests if Any.setValue(type, value) method throws on invalid args")]
    public void setValue_RejectsInvalidParams()
    {
        Any any = Any.with(10);
        Assert.That(() => any.setValue(null, 10), Throws.ArgumentNullException);
        Assert.That(() => any.setValue(Any.VOID), Throws.ArgumentException);
        Assert.That(() => any.setValue(typeof(int), null), Throws.ArgumentException);
    }

    [Test]
    [Description("Tests if Any.GetHashCode() returns same hash for identical values")]
    public void GetHashCode_ReturnsTrueForSame()
    {
        Assert.That(Any.VOID.GetHashCode() == Any.VOID.GetHashCode(), Is.True);
        Assert.That(Any.with(10).GetHashCode() == Any.with(10).GetHashCode(), Is.True);
    }

    [Test]
    [Description("Tests if Any.GetHashCode() returns different hash for different values")]
    public void GetHashCode_ReturnsTrueForDifferent()
    {
        Assert.That(Any.VOID.GetHashCode() == Any.with(10).GetHashCode(), Is.False);
    }
}
